#include "main_window.hpp"

#include "nebbie/edit.hpp"
#include "nebbie/io.hpp"
#include "nebbie/validate.hpp"

#include <QAction>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include <stdexcept>

namespace {

QWidget* makeEditorPage(QListWidget*& list_out, QWidget* editor) {
    auto* splitter = new QSplitter;
    list_out = new QListWidget;
    list_out->setMinimumWidth(180);
    splitter->addWidget(list_out);
    splitter->addWidget(editor);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);

    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(splitter);
    return page;
}

} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    setupMenus();
    setWindowTitle("Nebbie Editor");
    resize(960, 640);
    setStatus("Apri una libreria (mudroot/lib) per iniziare.");
}

void MainWindow::setupUi() {
    auto* central = new QWidget;
    auto* root_layout = new QVBoxLayout(central);

    lib_label_ = new QLabel("Nessuna libreria aperta");
    lib_label_->setWordWrap(true);
    root_layout->addWidget(lib_label_);

    tabs_ = new QTabWidget;

    auto* room_editor = new QWidget;
    auto* room_form = new QFormLayout(room_editor);
    room_name_ = new QLineEdit;
    room_desc_ = new QTextEdit;
    room_desc_->setMinimumHeight(120);
    room_sector_ = new QSpinBox;
    room_sector_->setRange(0, 999);
    room_flags_ = new QSpinBox;
    room_flags_->setRange(0, 2147483647);
    room_form->addRow("Nome:", room_name_);
    room_form->addRow("Descrizione:", room_desc_);
    room_form->addRow("Settore:", room_sector_);
    room_form->addRow("Flag:", room_flags_);
    auto* room_apply = new QPushButton("Applica modifiche");
    room_form->addRow(room_apply);
    tabs_->addTab(makeEditorPage(room_list_, room_editor), "Stanze");

    auto* mob_editor = new QWidget;
    auto* mob_form = new QFormLayout(mob_editor);
    mob_short_ = new QLineEdit;
    mob_long_ = new QTextEdit;
    mob_long_->setMinimumHeight(80);
    mob_level_ = new QSpinBox;
    mob_level_->setRange(0, 200);
    mob_alignment_ = new QSpinBox;
    mob_alignment_->setRange(-1000, 1000);
    mob_form->addRow("Nome breve:", mob_short_);
    mob_form->addRow("Descrizione lunga:", mob_long_);
    mob_form->addRow("Livello:", mob_level_);
    mob_form->addRow("Allineamento:", mob_alignment_);
    auto* mob_apply = new QPushButton("Applica modifiche");
    mob_form->addRow(mob_apply);
    tabs_->addTab(makeEditorPage(mob_list_, mob_editor), "Mob");

    auto* obj_editor = new QWidget;
    auto* obj_form = new QFormLayout(obj_editor);
    obj_short_ = new QLineEdit;
    obj_desc_ = new QTextEdit;
    obj_desc_->setMinimumHeight(80);
    obj_cost_ = new QSpinBox;
    obj_cost_->setRange(0, 1000000);
    obj_weight_ = new QSpinBox;
    obj_weight_->setRange(0, 100000);
    obj_form->addRow("Nome breve:", obj_short_);
    obj_form->addRow("Descrizione:", obj_desc_);
    obj_form->addRow("Costo:", obj_cost_);
    obj_form->addRow("Peso:", obj_weight_);
    auto* obj_apply = new QPushButton("Applica modifiche");
    obj_form->addRow(obj_apply);
    tabs_->addTab(makeEditorPage(obj_list_, obj_editor), "Oggetti");

    validation_log_ = new QPlainTextEdit;
    validation_log_->setReadOnly(true);
    validation_log_->setPlaceholderText("Esegui Valida per vedere errori e avvisi.");
    validation_tab_ = new QWidget;
    auto* validation_layout = new QVBoxLayout(validation_tab_);
    validation_layout->addWidget(new QLabel("Risultato validazione incrociata:"));
    validation_layout->addWidget(validation_log_);
    tabs_->addTab(validation_tab_, "Validazione");

    root_layout->addWidget(tabs_);
    setCentralWidget(central);
    statusBar()->showMessage("Pronto");

    connect(room_list_, &QListWidget::currentRowChanged, this, [this](int) { onRoomSelected(); });
    connect(mob_list_, &QListWidget::currentRowChanged, this, [this](int) { onMobSelected(); });
    connect(obj_list_, &QListWidget::currentRowChanged, this, [this](int) { onObjSelected(); });
    connect(room_apply, &QPushButton::clicked, this, &MainWindow::applyRoomChanges);
    connect(mob_apply, &QPushButton::clicked, this, &MainWindow::applyMobChanges);
    connect(obj_apply, &QPushButton::clicked, this, &MainWindow::applyObjChanges);
}

void MainWindow::setupMenus() {
    auto* file_menu = menuBar()->addMenu("&File");

    auto* open_action = file_menu->addAction("&Apri libreria...");
    open_action->setShortcut(QKeySequence::Open);
    connect(open_action, &QAction::triggered, this, &MainWindow::openLib);

    auto* save_action = file_menu->addAction("&Salva");
    save_action->setShortcut(QKeySequence::Save);
    connect(save_action, &QAction::triggered, this, &MainWindow::saveLib);

    auto* save_force_action = file_menu->addAction("Salva (forza)");
    connect(save_force_action, &QAction::triggered, this, &MainWindow::saveLibForce);

    file_menu->addSeparator();
    file_menu->addAction("E&sci", this, &QWidget::close);

    auto* tools_menu = menuBar()->addMenu("&Strumenti");
    auto* validate_action = tools_menu->addAction("&Valida");
    validate_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    connect(validate_action, &QAction::triggered, this, &MainWindow::validateLib);
}

void MainWindow::openLibPath(const QString& path) {
    if (path.isEmpty()) {
        return;
    }
    try {
        loadLib(std::filesystem::path(path.toStdString()));
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, "Errore", QString::fromUtf8(ex.what()));
    }
}

void MainWindow::openLib() {
    if (!confirmSaveIfDirty()) {
        return;
    }

    const QString dir = QFileDialog::getExistingDirectory(
        this, "Apri libreria Nebbie", QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) {
        return;
    }
    openLibPath(dir);
}

void MainWindow::loadLib(const std::filesystem::path& path) {
    world_.clear();
    context_ = {};
    nebbie::load_lib(world_, path, context_);
    lib_path_ = path;
    markClean();
    refreshLists();

    const QString label = QString("Libreria: %1 — %2 zone, %3 stanze, %4 mob, %5 oggetti")
                              .arg(QString::fromStdString(path.string()))
                              .arg(world_.zones.size())
                              .arg(world_.rooms.size())
                              .arg(world_.mobiles.size())
                              .arg(world_.objects.size());
    lib_label_->setText(label);
    setStatus("Libreria caricata.");
}

void MainWindow::refreshLists() {
    room_list_->clear();
    for (const auto& [vnum, room] : world_.rooms) {
        room_list_->addItem(QString("#%1 %2").arg(vnum).arg(QString::fromStdString(room.name)));
        room_list_->item(room_list_->count() - 1)->setData(Qt::UserRole, static_cast<qlonglong>(vnum));
    }

    mob_list_->clear();
    for (const auto& [vnum, mob] : world_.mobiles) {
        mob_list_->addItem(QString("#%1 %2").arg(vnum).arg(QString::fromStdString(mob.short_descr)));
        mob_list_->item(mob_list_->count() - 1)->setData(Qt::UserRole, static_cast<qlonglong>(vnum));
    }

    obj_list_->clear();
    for (const auto& [vnum, obj] : world_.objects) {
        obj_list_->addItem(QString("#%1 %2").arg(vnum).arg(QString::fromStdString(obj.short_descr)));
        obj_list_->item(obj_list_->count() - 1)->setData(Qt::UserRole, static_cast<qlonglong>(vnum));
    }
}

void MainWindow::onRoomSelected() {
    const auto* item = room_list_->currentItem();
    if (!item) {
        return;
    }
    const long vnum = static_cast<long>(item->data(Qt::UserRole).toLongLong());
    const nebbie::Room* room = world_.find_room(vnum);
    if (!room) {
        return;
    }
    room_name_->setText(QString::fromStdString(room->name));
    room_desc_->setPlainText(QString::fromStdString(room->description));
    room_sector_->setValue(static_cast<int>(room->sector_type));
    room_flags_->setValue(static_cast<int>(room->room_flags));
}

void MainWindow::onMobSelected() {
    const auto* item = mob_list_->currentItem();
    if (!item) {
        return;
    }
    const long vnum = static_cast<long>(item->data(Qt::UserRole).toLongLong());
    const nebbie::Mobile* mob = world_.find_mobile(vnum);
    if (!mob) {
        return;
    }
    mob_short_->setText(QString::fromStdString(mob->short_descr));
    mob_long_->setPlainText(QString::fromStdString(mob->long_descr));
    mob_level_->setValue(mob->level);
    mob_alignment_->setValue(static_cast<int>(mob->alignment));
}

void MainWindow::onObjSelected() {
    const auto* item = obj_list_->currentItem();
    if (!item) {
        return;
    }
    const long vnum = static_cast<long>(item->data(Qt::UserRole).toLongLong());
    const nebbie::GameObject* obj = world_.find_object(vnum);
    if (!obj) {
        return;
    }
    obj_short_->setText(QString::fromStdString(obj->short_descr));
    obj_desc_->setPlainText(QString::fromStdString(obj->description));
    obj_cost_->setValue(obj->cost);
    obj_weight_->setValue(obj->weight);
}

void MainWindow::applyRoomChanges() {
    auto* item = room_list_->currentItem();
    if (!item) {
        QMessageBox::information(this, "Stanze", "Seleziona una stanza.");
        return;
    }
    const long vnum = static_cast<long>(item->data(Qt::UserRole).toLongLong());

    nebbie::RoomEdit edit;
    edit.name = room_name_->text().toStdString();
    edit.description = room_desc_->toPlainText().toStdString();
    edit.sector_type = room_sector_->value();
    edit.room_flags = room_flags_->value();

    if (!nebbie::edit_room(world_, vnum, edit)) {
        QMessageBox::warning(this, "Stanze", "Stanza non trovata.");
        return;
    }

    item->setText(QString("#%1 %2").arg(vnum).arg(room_name_->text()));
    markDirty();
    setStatus(QString("Stanza %1 aggiornata in memoria.").arg(vnum));
}

void MainWindow::applyMobChanges() {
    auto* item = mob_list_->currentItem();
    if (!item) {
        QMessageBox::information(this, "Mob", "Seleziona un mobile.");
        return;
    }
    const long vnum = static_cast<long>(item->data(Qt::UserRole).toLongLong());

    nebbie::MobEdit edit;
    edit.short_descr = mob_short_->text().toStdString();
    edit.long_descr = mob_long_->toPlainText().toStdString();
    edit.level = mob_level_->value();
    edit.alignment = mob_alignment_->value();

    if (!nebbie::edit_mob(world_, vnum, edit)) {
        QMessageBox::warning(this, "Mob", "Mobile non trovato.");
        return;
    }

    item->setText(QString("#%1 %2").arg(vnum).arg(mob_short_->text()));
    markDirty();
    setStatus(QString("Mobile %1 aggiornato in memoria.").arg(vnum));
}

void MainWindow::applyObjChanges() {
    auto* item = obj_list_->currentItem();
    if (!item) {
        QMessageBox::information(this, "Oggetti", "Seleziona un oggetto.");
        return;
    }
    const long vnum = static_cast<long>(item->data(Qt::UserRole).toLongLong());

    nebbie::ObjEdit edit;
    edit.short_descr = obj_short_->text().toStdString();
    edit.description = obj_desc_->toPlainText().toStdString();
    edit.cost = obj_cost_->value();
    edit.weight = obj_weight_->value();

    if (!nebbie::edit_object(world_, vnum, edit)) {
        QMessageBox::warning(this, "Oggetti", "Oggetto non trovato.");
        return;
    }

    item->setText(QString("#%1 %2").arg(vnum).arg(obj_short_->text()));
    markDirty();
    setStatus(QString("Oggetto %1 aggiornato in memoria.").arg(vnum));
}

void MainWindow::showValidation(const nebbie::ValidationReport& report) {
    QString text;
    for (const auto& issue : report.issues) {
        const char* level = issue.severity == nebbie::ValidationSeverity::error ? "ERRORE" : "AVVISO";
        text += QString("[%1] %2: %3\n")
                    .arg(level)
                    .arg(QString::fromStdString(issue.category))
                    .arg(QString::fromStdString(issue.message));
    }
    text += QString("\n%1 errori, %2 avvisi")
                .arg(report.error_count())
                .arg(report.warning_count());
    validation_log_->setPlainText(text);
    tabs_->setCurrentWidget(validation_tab_);
}

void MainWindow::validateLib() {
    if (lib_path_.empty()) {
        QMessageBox::information(this, "Valida", "Apri prima una libreria.");
        return;
    }
    const nebbie::ValidationReport report = nebbie::validate_world(world_);
    showValidation(report);
    if (report.ok()) {
        setStatus("Validazione OK.");
    } else {
        setStatus(QString("Validazione: %1 errori.").arg(report.error_count()));
    }
}

void MainWindow::saveLib() {
    if (lib_path_.empty()) {
        QMessageBox::information(this, "Salva", "Apri prima una libreria.");
        return;
    }

    const nebbie::ValidationReport report = nebbie::validate_world(world_);
    if (!report.ok()) {
        showValidation(report);
        const auto answer = QMessageBox::question(
            this, "Errori di validazione",
            QString("Ci sono %1 errori. Salvare comunque?").arg(report.error_count()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (answer != QMessageBox::Yes) {
            return;
        }
    }

    try {
        nebbie::save_lib(world_, context_);
        markClean();
        setStatus("Libreria salvata.");
        QMessageBox::information(this, "Salva", "Salvataggio completato.");
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, "Errore", QString::fromUtf8(ex.what()));
    }
}

void MainWindow::saveLibForce() {
    if (lib_path_.empty()) {
        QMessageBox::information(this, "Salva", "Apri prima una libreria.");
        return;
    }
    try {
        nebbie::save_lib(world_, context_);
        markClean();
        setStatus("Libreria salvata (forzato).");
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, "Errore", QString::fromUtf8(ex.what()));
    }
}

void MainWindow::setStatus(const QString& message) {
    statusBar()->showMessage(message);
}

void MainWindow::markDirty() {
    dirty_ = true;
    setWindowTitle("Nebbie Editor *");
}

void MainWindow::markClean() {
    dirty_ = false;
    setWindowTitle("Nebbie Editor");
}

bool MainWindow::confirmSaveIfDirty() {
    if (!dirty_) {
        return true;
    }
    const auto answer = QMessageBox::question(
        this, "Modifiche non salvate",
        "Ci sono modifiche non salvate. Salvare prima di continuare?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (answer == QMessageBox::Save) {
        saveLib();
        return !dirty_;
    }
    if (answer == QMessageBox::Discard) {
        return true;
    }
    return false;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (!confirmSaveIfDirty()) {
        event->ignore();
        return;
    }
    event->accept();
}

#include "main_window.hpp"

#include "app_config.hpp"
#include "world_zone_map_widget.hpp"
#include "zone_map_widget.hpp"
#include "nebbie/edit.hpp"
#include "nebbie/io.hpp"
#include "nebbie/validate.hpp"
#include "nebbie/zone_graph.hpp"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QCheckBox>
#include <QTabWidget>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QDateTime>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include <stdexcept>

namespace {

struct EntityPageWidgets {
    QWidget* page = nullptr;
    QLineEdit* search = nullptr;
    QPushButton* create_button = nullptr;
    QListWidget* list = nullptr;
};

EntityPageWidgets makeEntityPage(QWidget* editor, const QString& create_label) {
    EntityPageWidgets widgets;
    widgets.page = new QWidget;
    auto* layout = new QVBoxLayout(widgets.page);

    auto* top = new QHBoxLayout;
    widgets.search = new QLineEdit;
    widgets.search->setPlaceholderText("Cerca vnum o nome...");
    widgets.create_button = new QPushButton(create_label);
    top->addWidget(widgets.search, 1);
    top->addWidget(widgets.create_button);
    layout->addLayout(top);

    auto* splitter = new QSplitter;
    widgets.list = new QListWidget;
    widgets.list->setMinimumWidth(180);
    splitter->addWidget(widgets.list);
    splitter->addWidget(editor);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);
    layout->addWidget(splitter, 1);
    return widgets;
}

void addListItem(QListWidget* list, long vnum, const QString& label) {
    list->addItem(label);
    list->item(list->count() - 1)->setData(Qt::UserRole, static_cast<qlonglong>(vnum));
}

} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    setupMenus();
    setWindowTitle("Nebbie Editor");
    resize(1100, 720);
    setStatus("Apri una libreria (mudroot/lib) per iniziare.");
}

void MainWindow::setupUi() {
    auto* central = new QWidget;
    auto* root_layout = new QVBoxLayout(central);

    lib_label_ = new QLabel("Nessuna libreria aperta");
    lib_label_->setWordWrap(true);
    root_layout->addWidget(lib_label_);

    tabs_ = new QTabWidget;

    auto* room_scroll = new QScrollArea;
    room_scroll->setWidgetResizable(true);
    auto* room_editor = new QWidget;
    auto* room_layout = new QVBoxLayout(room_editor);

    auto* room_form = new QFormLayout;
    room_name_ = new QLineEdit;
    room_desc_ = new QTextEdit;
    room_desc_->setMinimumHeight(100);
    room_sector_ = new QSpinBox;
    room_sector_->setRange(0, 999);
    room_flags_ = new QSpinBox;
    room_flags_->setRange(0, 2147483647);
    room_form->addRow("Nome:", room_name_);
    room_form->addRow("Descrizione:", room_desc_);
    room_form->addRow("Settore:", room_sector_);
    room_form->addRow("Flag:", room_flags_);
    auto* room_apply = new QPushButton("Applica modifiche stanza");
    room_form->addRow(room_apply);
    room_layout->addLayout(room_form);

    auto* exit_group = new QGroupBox("Uscite (mappatura)");
    auto* exit_layout = new QVBoxLayout(exit_group);
    exit_list_ = new QListWidget;
    exit_list_->setMaximumHeight(120);
    exit_layout->addWidget(exit_list_);

    auto* exit_form = new QFormLayout;
    exit_direction_ = new QComboBox;
    for (int i = 0; i < nebbie::EXIT_DIR_COUNT; ++i) {
        exit_direction_->addItem(QString::fromUtf8(nebbie::exit_direction_name(i)), i);
    }
    exit_to_room_ = new QSpinBox;
    exit_to_room_->setRange(0, 999999);
    exit_description_ = new QLineEdit;
    exit_keyword_ = new QLineEdit;
    exit_info_ = new QSpinBox;
    exit_info_->setRange(0, 2147483647);
    exit_key_ = new QSpinBox;
    exit_key_->setRange(-1, 999999);
    exit_form->addRow("Direzione:", exit_direction_);
    exit_form->addRow("Verso stanza #:", exit_to_room_);
    exit_form->addRow("Descrizione:", exit_description_);
    exit_form->addRow("Parola chiave:", exit_keyword_);
    exit_form->addRow("Flag uscita:", exit_info_);
    exit_form->addRow("Chiave (vnum):", exit_key_);
    exit_layout->addLayout(exit_form);

    auto* exit_buttons = new QHBoxLayout;
    auto* exit_apply = new QPushButton("Aggiungi / aggiorna uscita");
    auto* exit_remove = new QPushButton("Elimina uscita");
    auto* exit_goto = new QPushButton("Vai alla stanza");
    exit_buttons->addWidget(exit_apply);
    exit_buttons->addWidget(exit_remove);
    exit_buttons->addWidget(exit_goto);
    exit_layout->addLayout(exit_buttons);
    room_layout->addWidget(exit_group);
    room_scroll->setWidget(room_editor);

    const EntityPageWidgets room_page = makeEntityPage(room_scroll, "Nuova stanza");
    room_search_ = room_page.search;
    room_list_ = room_page.list;
    tabs_->addTab(room_page.page, "Stanze");

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
    const EntityPageWidgets mob_page = makeEntityPage(mob_editor, "Nuovo mob");
    mob_search_ = mob_page.search;
    mob_list_ = mob_page.list;
    tabs_->addTab(mob_page.page, "Mob");

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
    const EntityPageWidgets obj_page = makeEntityPage(obj_editor, "Nuovo oggetto");
    obj_search_ = obj_page.search;
    obj_list_ = obj_page.list;
    tabs_->addTab(obj_page.page, "Oggetti");

    auto* zone_page = new QWidget;
    auto* zone_layout = new QVBoxLayout(zone_page);
    auto* zone_splitter = new QSplitter;

    auto* zone_left = new QWidget;
    auto* zone_left_layout = new QVBoxLayout(zone_left);
    zone_left_layout->addWidget(new QLabel("Zone"));
    zone_list_ = new QListWidget;
    zone_list_->setMinimumWidth(160);
    zone_left_layout->addWidget(zone_list_);
    zone_splitter->addWidget(zone_left);

    auto* zone_right = new QWidget;
    auto* zone_right_layout = new QVBoxLayout(zone_right);
    zone_info_ = new QLabel("Seleziona una zona");
    zone_info_->setWordWrap(true);
    zone_right_layout->addWidget(zone_info_);

    zone_right_layout->addWidget(new QLabel("Reset (myst.zon)"));
    reset_list_ = new QListWidget;
    reset_list_->setMinimumHeight(140);
    zone_right_layout->addWidget(reset_list_);

    auto* reset_form = new QFormLayout;
    reset_command_ = new QComboBox;
    for (const char cmd : {'M', 'O', 'G', 'E', 'P', 'D', 'C', 'H'}) {
        reset_command_->addItem(QString(QChar(cmd)), QChar(cmd));
    }
    reset_hint_ = new QLabel;
    reset_hint_->setWordWrap(true);
    reset_if_flag_ = new QSpinBox;
    reset_if_flag_->setRange(0, 999);
    reset_arg1_ = new QSpinBox;
    reset_arg1_->setRange(-1, 999999);
    reset_arg2_ = new QSpinBox;
    reset_arg2_->setRange(-1, 999999);
    reset_arg3_ = new QSpinBox;
    reset_arg3_->setRange(-1, 999999);
    reset_arg4_ = new QSpinBox;
    reset_arg4_->setRange(-1, 999999);
    reset_form->addRow("Comando:", reset_command_);
    reset_form->addRow("Guida:", reset_hint_);
    reset_form->addRow("if_flag:", reset_if_flag_);
    reset_form->addRow("arg1:", reset_arg1_);
    reset_form->addRow("arg2:", reset_arg2_);
    reset_form->addRow("arg3:", reset_arg3_);
    reset_form->addRow("arg4:", reset_arg4_);
    zone_right_layout->addLayout(reset_form);

    auto* reset_buttons = new QHBoxLayout;
    auto* reset_add = new QPushButton("Nuovo reset");
    reset_apply_ = new QPushButton("Applica");
    reset_remove_ = new QPushButton("Elimina");
    auto* reset_up = new QPushButton("Su");
    auto* reset_down = new QPushButton("Giu");
    auto* reset_goto_room = new QPushButton("Vai stanza");
    auto* reset_goto_entity = new QPushButton("Vai entita");
    reset_buttons->addWidget(reset_add);
    reset_buttons->addWidget(reset_apply_);
    reset_buttons->addWidget(reset_remove_);
    reset_buttons->addWidget(reset_up);
    reset_buttons->addWidget(reset_down);
    reset_buttons->addWidget(reset_goto_room);
    reset_buttons->addWidget(reset_goto_entity);
    zone_right_layout->addLayout(reset_buttons);
    zone_splitter->addWidget(zone_right);
    zone_splitter->setStretchFactor(0, 1);
    zone_splitter->setStretchFactor(1, 3);
    zone_layout->addWidget(zone_splitter);
    zone_tab_ = zone_page;
    tabs_->addTab(zone_page, "Zone");

    map_tab_ = new QWidget;
    auto* map_layout = new QVBoxLayout(map_tab_);
    auto* map_tabs = new QTabWidget;

    auto* map_zone_page = new QWidget;
    auto* map_zone_layout = new QVBoxLayout(map_zone_page);
    map_zone_layout->addWidget(
        new QLabel("Stanze della zona. Piano Z separato; clic su su/giù cambia piano ed evidenzia la destinazione."));
    auto* map_top = new QHBoxLayout;
    map_zone_ = new QComboBox;
    map_floor_ = new QComboBox;
    map_broken_only_ = new QCheckBox("Solo link rotti");
    auto* map_refresh = new QPushButton("Aggiorna");
    auto* map_export_dot = new QPushButton("Esporta DOT");
    map_top->addWidget(new QLabel("Zona:"));
    map_top->addWidget(map_zone_, 1);
    map_top->addWidget(new QLabel("Piano Z:"));
    map_top->addWidget(map_floor_);
    map_top->addWidget(map_broken_only_);
    map_top->addWidget(map_refresh);
    map_top->addWidget(map_export_dot);
    map_zone_layout->addLayout(map_top);

    map_view_ = new ZoneMapWidget;
    map_view_->setMinimumHeight(320);
    map_zone_layout->addWidget(map_view_, 1);

    map_stats_ = new QLabel;
    map_stats_->setWordWrap(true);
    map_zone_layout->addWidget(map_stats_);
    map_tabs->addTab(map_zone_page, "Zona");

    auto* map_world_page = new QWidget;
    auto* map_world_layout = new QVBoxLayout(map_world_page);
    map_world_layout->addWidget(
        new QLabel("Collegamenti tra zone (non stanze). U=vnum usati, L=vnum liberi nel range. Clic seleziona, doppio clic apre tab Zone."));
    auto* world_top = new QHBoxLayout;
    world_map_broken_only_ = new QCheckBox("Solo link rotti");
    auto* world_refresh = new QPushButton("Aggiorna");
    auto* world_export_dot = new QPushButton("Esporta DOT");
    world_top->addWidget(world_map_broken_only_);
    world_top->addStretch(1);
    world_top->addWidget(world_refresh);
    world_top->addWidget(world_export_dot);
    map_world_layout->addLayout(world_top);

    world_map_view_ = new WorldZoneMapWidget;
    world_map_view_->setMinimumHeight(320);
    map_world_layout->addWidget(world_map_view_, 2);

    world_map_details_ = new QPlainTextEdit;
    world_map_details_->setReadOnly(true);
    world_map_details_->setPlaceholderText("Clic su una zona per vedere vnum usati e liberi.");
    world_map_details_->setMaximumHeight(140);
    map_world_layout->addWidget(world_map_details_, 1);

    world_map_stats_ = new QLabel;
    world_map_stats_->setWordWrap(true);
    map_world_layout->addWidget(world_map_stats_);
    map_tabs->addTab(map_world_page, "Mondo (zone)");

    map_layout->addWidget(map_tabs, 1);
    tabs_->addTab(map_tab_, "Mappa");

    validation_tab_ = new QWidget;
    auto* validation_layout = new QVBoxLayout(validation_tab_);

    auto* validation_top = new QHBoxLayout;
    auto* validate_button = new QPushButton("Valida ora");
    auto* validation_summary = new QLabel("Esegui Valida per vedere errori e avvisi. Doppio clic per andare all'entità.");
    validation_summary->setWordWrap(true);
    validation_top->addWidget(validate_button);
    validation_layout->addWidget(validation_summary);
    validation_layout->addLayout(validation_top);

    validation_list_ = new QListWidget;
    validation_layout->addWidget(validation_list_, 1);
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
    connect(room_page.create_button, &QPushButton::clicked, this, &MainWindow::createRoom);
    connect(mob_page.create_button, &QPushButton::clicked, this, &MainWindow::createMob);
    connect(obj_page.create_button, &QPushButton::clicked, this, &MainWindow::createObject);
    connect(room_search_, &QLineEdit::textChanged, this, &MainWindow::onRoomSearchChanged);
    connect(mob_search_, &QLineEdit::textChanged, this, &MainWindow::onMobSearchChanged);
    connect(obj_search_, &QLineEdit::textChanged, this, &MainWindow::onObjSearchChanged);
    connect(exit_list_, &QListWidget::currentRowChanged, this, [this](int) { onExitSelected(); });
    connect(exit_apply, &QPushButton::clicked, this, &MainWindow::applyExitChanges);
    connect(exit_remove, &QPushButton::clicked, this, &MainWindow::removeSelectedExit);
    connect(exit_goto, &QPushButton::clicked, this, &MainWindow::goToExitTarget);
    connect(exit_list_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) {
        goToExitTarget();
    });
    connect(zone_list_, &QListWidget::currentRowChanged, this, [this](int) { onZoneSelected(); });
    connect(reset_list_, &QListWidget::currentRowChanged, this, [this](int) { onResetSelected(); });
    connect(reset_command_, &QComboBox::currentIndexChanged, this, &MainWindow::onResetCommandChanged);
    connect(reset_add, &QPushButton::clicked, this, &MainWindow::addZoneReset);
    connect(reset_apply_, &QPushButton::clicked, this, &MainWindow::applyResetChanges);
    connect(reset_remove_, &QPushButton::clicked, this, &MainWindow::removeSelectedReset);
    connect(reset_up, &QPushButton::clicked, this, &MainWindow::moveResetUp);
    connect(reset_down, &QPushButton::clicked, this, &MainWindow::moveResetDown);
    connect(reset_goto_room, &QPushButton::clicked, this, &MainWindow::goToResetRoom);
    connect(reset_goto_entity, &QPushButton::clicked, this, &MainWindow::goToResetEntity);
    connect(validate_button, &QPushButton::clicked, this, &MainWindow::validateLib);
    connect(validation_list_, &QListWidget::itemDoubleClicked, this, &MainWindow::onValidationIssueActivated);
    connect(map_refresh, &QPushButton::clicked, this, &MainWindow::refreshZoneMap);
    connect(map_zone_, &QComboBox::currentIndexChanged, this, [this](int) { refreshZoneMap(); });
    connect(map_broken_only_, &QCheckBox::toggled, this, [this](bool enabled) {
        if (map_view_) {
            map_view_->setShowBrokenOnly(enabled);
            updateMapStats();
        }
    });
    connect(map_floor_, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index < 0 || !map_view_) {
            return;
        }
        map_view_->setActiveZLevel(map_floor_->currentData().toInt());
        updateMapStats();
    });
    connect(map_view_, &ZoneMapWidget::roomActivated, this, [this](long vnum) {
        tabs_->setCurrentIndex(0);
        selectRoomByVnum(vnum);
        setStatus(QString("Mappa: selezionata stanza #%1.").arg(vnum));
    });
    connect(map_view_, &ZoneMapWidget::floorLinkActivated, this, [this](long target_vnum, int target_z) {
        const int index = map_floor_->findData(target_z);
        if (index >= 0) {
            map_floor_->setCurrentIndex(index);
        } else {
            map_view_->setActiveZLevel(target_z);
        }
        map_view_->setHighlightedVnum(target_vnum);
        setStatus(QString("Mappa: piano Z=%1, evidenciata stanza #%2.").arg(target_z).arg(target_vnum));
    });
    connect(map_export_dot, &QPushButton::clicked, this, [this]() {
        if (map_zone_->count() == 0) {
            return;
        }
        const int zone_num = map_zone_->currentData().toInt();
        const nebbie::ZoneGraph graph = nebbie::build_zone_graph(world_, zone_num);
        QApplication::clipboard()->setText(QString::fromStdString(nebbie::zone_graph_to_dot(graph)));
        setStatus("DOT zona copiato negli appunti.");
    });
    connect(world_refresh, &QPushButton::clicked, this, &MainWindow::refreshWorldZoneMap);
    connect(world_map_broken_only_, &QCheckBox::toggled, this, [this](bool enabled) {
        if (world_map_view_) {
            world_map_view_->setShowBrokenOnly(enabled);
        }
    });
    connect(world_map_view_, &WorldZoneMapWidget::zoneSelected, this, [this](int zone_num) {
        updateWorldZoneDetails(zone_num);
    });
    connect(world_map_view_, &WorldZoneMapWidget::zoneActivated, this, [this](int zone_num) {
        if (zone_tab_) {
            tabs_->setCurrentWidget(zone_tab_);
        }
        selectZoneByNum(zone_num);
        updateWorldZoneDetails(zone_num);
        setStatus(QString("Mappa mondo: selezionata zona #%1.").arg(zone_num));
    });
    connect(world_export_dot, &QPushButton::clicked, this, [this]() {
        const nebbie::WorldZoneGraph graph = nebbie::build_world_zone_graph(world_);
        QApplication::clipboard()->setText(QString::fromStdString(nebbie::world_zone_graph_to_dot(graph)));
        setStatus("DOT mondo (zone) copiato negli appunti.");
    });
    updateResetFieldHints();

    autosave_timer_ = new QTimer(this);
    autosave_timer_->setInterval(session_config_.autosave_interval_sec * 1000);
    connect(autosave_timer_, &QTimer::timeout, this, &MainWindow::onAutosaveTick);
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

    auto* history_menu = file_menu->addMenu("Cronologia");
    auto* restore_workspace_action = history_menu->addAction("Ripristina ultimo autosalvataggio");
    connect(restore_workspace_action, &QAction::triggered, this, &MainWindow::restoreFromWorkspace);
    auto* restore_version_action = history_menu->addAction("Ripristina versione...");
    connect(restore_version_action, &QAction::triggered, this, &MainWindow::restoreVersion);

    file_menu->addSeparator();
    file_menu->addAction("E&sci", this, &QWidget::close);

    auto* tools_menu = menuBar()->addMenu("&Strumenti");
    auto* validate_action = tools_menu->addAction("&Valida");
    validate_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    connect(validate_action, &QAction::triggered, this, &MainWindow::validateLib);
}

void MainWindow::rememberLibPath(const std::filesystem::path& path) {
    nebbie::qt::write_lib_path(QString::fromStdString(path.string()));
}

void MainWindow::openLibPath(const QString& path) {
    if (path.isEmpty()) {
        return;
    }
    try {
        const std::filesystem::path requested(path.toStdString());
        const std::filesystem::path resolved = nebbie::resolve_lib_directory(requested);
        loadLib(resolved);
        rememberLibPath(resolved);
        if (resolved != requested) {
            setStatus(QString("Libreria risolta in: %1").arg(QString::fromStdString(resolved.string())));
        }
    } catch (const std::exception& ex) {
        const std::filesystem::path resolved = nebbie::resolve_lib_directory(path.toStdString());
        const QString detail = QString::fromUtf8(ex.what())
                               + QString("\n\nPercorso richiesto: %1").arg(path)
                               + QString("\nPercorso risolto: %1").arg(QString::fromStdString(resolved.string()))
                               + "\n\nSeleziona la cartella mudroot/lib e ricompila la GUI:\n"
                                 "  ./scripts/build.sh";
        QMessageBox::critical(this, "Errore caricamento libreria", detail);
    }
}

void MainWindow::openLib() {
    if (!confirmSaveIfDirty()) {
        return;
    }

    const QString dir = QFileDialog::getExistingDirectory(
        this, "Apri libreria Nebbie (mudroot o mudroot/lib)", QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) {
        return;
    }
    openLibPath(dir);
}

void MainWindow::openStartupLib() {
    const QString saved = nebbie::qt::read_lib_path();
    if (nebbie::qt::lib_path_exists(saved)) {
        openLibPath(saved);
        return;
    }

    if (!saved.isEmpty()) {
        promptForLibPath(QString("Il percorso salvato non è più valido:\n%1").arg(saved));
        return;
    }

    promptForLibPath(QString(
        "Benvenuto in Nebbie Editor.\n\n"
        "Seleziona la cartella della libreria di gioco (mudroot o mudroot/lib).\n"
        "Il percorso verrà salvato in:\n%1")
                         .arg(nebbie::qt::default_config_path()));
}

bool MainWindow::promptForLibPath(const QString& reason) {
    if (!reason.isEmpty()) {
        QMessageBox::information(this, "Libreria Nebbie", reason);
    }

    const QString initial = nebbie::qt::read_lib_path();
    const QString dir = QFileDialog::getExistingDirectory(
        this,
        "Seleziona libreria Nebbie (mudroot o mudroot/lib)",
        initial.isEmpty() ? QDir::homePath() : initial,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) {
        setStatus("Nessuna libreria selezionata. Usa File → Apri libreria.");
        return false;
    }

    openLibPath(dir);
    return !lib_path_.empty();
}

void MainWindow::loadLib(const std::filesystem::path& path) {
    world_.clear();
    context_ = {};
    nebbie::load_lib(world_, path, context_);
    lib_path_ = path;
    room_filter_.clear();
    mob_filter_.clear();
    object_filter_.clear();
    room_search_->clear();
    mob_search_->clear();
    obj_search_->clear();
    markClean();
    refreshRoomList();
    refreshMobList();
    refreshObjectList();
    refreshZoneList();
    refreshZoneMap();
    refreshWorldZoneMap();

    const QString label = QString("Libreria: %1 — %2 zone, %3 stanze, %4 mob, %5 oggetti")
                              .arg(QString::fromStdString(path.string()))
                              .arg(world_.zones.size())
                              .arg(world_.rooms.size())
                              .arg(world_.mobiles.size())
                              .arg(world_.objects.size());
    lib_label_->setText(label);
    last_version_time_ = std::chrono::system_clock::now();
    autosave_timer_->start();
    setStatus("Libreria caricata.");
}

void MainWindow::refreshRoomList() {
    const long selected = currentRoomVnum();
    room_list_->clear();
    const std::string query = room_filter_.toStdString();
    for (const auto& [vnum, room] : world_.rooms) {
        if (!nebbie::entity_matches(vnum, room.name, query)) {
            continue;
        }
        addListItem(room_list_, vnum,
                    QString("#%1 %2").arg(vnum).arg(QString::fromStdString(room.name)));
    }
    if (selected > 0) {
        selectRoomByVnum(selected);
    }
}

void MainWindow::refreshMobList() {
    mob_list_->clear();
    const std::string query = mob_filter_.toStdString();
    for (const auto& [vnum, mob] : world_.mobiles) {
        if (!nebbie::entity_matches(vnum, mob.short_descr, query)) {
            continue;
        }
        addListItem(mob_list_, vnum,
                    QString("#%1 %2").arg(vnum).arg(QString::fromStdString(mob.short_descr)));
    }
}

void MainWindow::refreshObjectList() {
    obj_list_->clear();
    const std::string query = object_filter_.toStdString();
    for (const auto& [vnum, obj] : world_.objects) {
        if (!nebbie::entity_matches(vnum, obj.short_descr, query)) {
            continue;
        }
        addListItem(obj_list_, vnum,
                    QString("#%1 %2").arg(vnum).arg(QString::fromStdString(obj.short_descr)));
    }
}

void MainWindow::refreshZoneList() {
    zone_list_->clear();
    for (const auto& zone : world_.zones) {
        const QString label = QString("#%1 %2 [%3-%4]")
                                  .arg(zone.num)
                                  .arg(QString::fromStdString(zone.name))
                                  .arg(zone.bottom)
                                  .arg(zone.top);
        addListItem(zone_list_, zone.num, label);
    }
    if (zone_list_->count() > 0 && zone_list_->currentRow() < 0) {
        zone_list_->setCurrentRow(0);
    }

    const int previous_zone = map_zone_->currentData().toInt();
    map_zone_->clear();
    for (const auto& zone : world_.zones) {
        map_zone_->addItem(QString("#%1 %2").arg(zone.num).arg(QString::fromStdString(zone.name)), zone.num);
    }
    if (map_zone_->count() > 0) {
        int index = map_zone_->findData(previous_zone);
        if (index < 0) {
            index = 0;
        }
        map_zone_->setCurrentIndex(index);
    }
}

void MainWindow::refreshZoneMap() {
    if (!map_view_ || map_zone_->count() == 0) {
        return;
    }

    const int zone_num = map_zone_->currentData().toInt();
    const nebbie::ZoneGraph graph = nebbie::build_zone_graph(world_, zone_num);
    if (graph.nodes.empty()) {
        map_view_->clearGraph();
        map_floor_->clear();
        if (map_stats_) {
            map_stats_->setText("Nessuna stanza in questa zona.");
        }
        return;
    }

    map_view_->setGraph(graph);

    const int previous_floor = map_floor_->currentData().toInt();
    map_floor_->clear();
    for (int level : map_view_->availableZLevels()) {
        map_floor_->addItem(QString("Z = %1").arg(level), level);
    }
    int floor_index = map_floor_->findData(previous_floor);
    if (floor_index < 0) {
        floor_index = 0;
    }
    map_floor_->setCurrentIndex(floor_index);
    map_view_->setActiveZLevel(map_floor_->currentData().toInt());
    if (map_broken_only_) {
        map_view_->setShowBrokenOnly(map_broken_only_->isChecked());
    }
    updateMapStats();
}

void MainWindow::refreshWorldZoneMap() {
    if (!world_map_view_) {
        return;
    }

    const nebbie::WorldZoneGraph graph = nebbie::build_world_zone_graph(world_);
    if (graph.zones.empty()) {
        world_map_view_->clearGraph();
        if (world_map_details_) {
            world_map_details_->clear();
        }
        if (world_map_stats_) {
            world_map_stats_->setText("Nessuna zona caricata.");
        }
        return;
    }

    world_map_view_->setGraph(graph);
    if (world_map_broken_only_) {
        world_map_view_->setShowBrokenOnly(world_map_broken_only_->isChecked());
    }

    int total_used = 0;
    int total_free = 0;
    int broken_edges = 0;
    for (const auto& zone : graph.zones) {
        total_used += zone.used_count;
        total_free += zone.free_count;
    }
    for (const auto& edge : graph.edges) {
        if (edge.broken_count > 0) {
            ++broken_edges;
        }
    }

    if (world_map_stats_) {
        world_map_stats_->setText(
            QString("Zone: %1 | collegamenti inter-zona: %2 | link rotti: %3 | vnum stanza usati: %4 | vnum liberi: %5")
                .arg(graph.zones.size())
                .arg(graph.edges.size())
                .arg(broken_edges)
                .arg(total_used)
                .arg(total_free));
    }
}

void MainWindow::updateWorldZoneDetails(int zone_num) {
    if (!world_map_details_) {
        return;
    }

    const nebbie::Zone* zone = nebbie::find_zone(world_, zone_num);
    if (!zone) {
        world_map_details_->setPlainText(QString("Zona #%1 non trovata.").arg(zone_num));
        return;
    }

    const nebbie::WorldZoneNode node = nebbie::build_world_zone_node(world_, *zone);
    QString used_text;
    if (node.used_vnums.empty()) {
        used_text = "(nessuna stanza nel range)";
    } else if (node.used_vnums.size() <= 40) {
        QStringList parts;
        for (long vnum : node.used_vnums) {
            parts << QString::number(vnum);
        }
        used_text = parts.join(", ");
    } else {
        used_text = QString("%1 … %2 (%3 vnum, elenco troncato)")
                        .arg(node.used_vnums.front())
                        .arg(node.used_vnums.back())
                        .arg(node.used_count);
    }

    const QString details = QString("Zona #%1 %2\nRange vnum: %3-%4\n\nStanze usate (%5):\n%6\n\nVnum liberi (%7):\n%8")
                                .arg(node.zone_num)
                                .arg(QString::fromStdString(node.name))
                                .arg(node.bottom)
                                .arg(node.top)
                                .arg(node.used_count)
                                .arg(used_text)
                                .arg(node.free_count)
                                .arg(QString::fromStdString(nebbie::format_vnum_ranges(node.free_ranges, 20)));
    world_map_details_->setPlainText(details);
    if (world_map_view_) {
        world_map_view_->setHighlightedZone(zone_num);
    }
}

void MainWindow::updateMapStats() {
    if (!map_stats_ || map_zone_->count() == 0) {
        return;
    }

    const int zone_num = map_zone_->currentData().toInt();
    const nebbie::ZoneGraph graph = nebbie::build_zone_graph(world_, zone_num);
    if (graph.nodes.empty()) {
        return;
    }

    const nebbie::ZoneZLayout z_layout = nebbie::compute_zone_z_levels(graph);
    const int active_z = map_view_ ? map_view_->activeZLevel() : 0;

    std::size_t on_floor = 0;
    std::size_t broken = 0;
    std::size_t vertical = 0;
    for (const auto& node : graph.nodes) {
        const auto it = z_layout.levels.find(node.vnum);
        if (it != z_layout.levels.end() && it->second == active_z) {
            ++on_floor;
        }
    }
    for (const auto& edge : graph.edges) {
        if (edge.broken) {
            ++broken;
        }
        if (edge.direction >= 4) {
            ++vertical;
        }
    }

    map_stats_->setText(QString("Zona %1 %2 [%3-%4] — piano Z=%5: %6 stanze | totali: %7 | archi: %8 | su/giù: %9 | rotti: %10")
                            .arg(graph.zone_num)
                            .arg(QString::fromStdString(graph.zone_name))
                            .arg(graph.bottom)
                            .arg(graph.top)
                            .arg(active_z)
                            .arg(on_floor)
                            .arg(graph.nodes.size())
                            .arg(graph.edges.size())
                            .arg(vertical)
                            .arg(broken));
}

void MainWindow::refreshResetList(const int zone_num) {
    reset_list_->clear();
    const nebbie::Zone* zone = nebbie::find_zone(world_, zone_num);
    if (!zone) {
        return;
    }

    for (std::size_t i = 0; i < zone->commands.size(); ++i) {
        const auto& cmd = zone->commands[i];
        const QString label = QString("[%1] %2")
                                  .arg(i)
                                  .arg(QString::fromStdString(nebbie::reset_command_summary(cmd)));
        reset_list_->addItem(label);
        reset_list_->item(static_cast<int>(i))->setData(Qt::UserRole, static_cast<qlonglong>(i));
        if (!nebbie::is_editable_reset_command(cmd.command)) {
            reset_list_->item(static_cast<int>(i))->setFlags(Qt::NoItemFlags);
        }
    }
}

int MainWindow::currentZoneNum() const {
    const auto* item = zone_list_->currentItem();
    if (!item) {
        return 0;
    }
    return static_cast<int>(item->data(Qt::UserRole).toLongLong());
}

int MainWindow::currentResetIndex() const {
    const auto* item = reset_list_->currentItem();
    if (!item) {
        return -1;
    }
    return static_cast<int>(item->data(Qt::UserRole).toLongLong());
}

void MainWindow::selectZoneByNum(const int zone_num) {
    for (int i = 0; i < zone_list_->count(); ++i) {
        if (zone_list_->item(i)->data(Qt::UserRole).toLongLong() == zone_num) {
            zone_list_->setCurrentRow(i);
            tabs_->setCurrentWidget(zone_tab_);
            return;
        }
    }
}

void MainWindow::updateResetFieldHints() {
    const QChar cmd = reset_command_->currentData().toChar();
    QString hint;
    switch (cmd.toLatin1()) {
    case 'M':
        hint = "Mobile: if_flag | mob vnum | max in stanza | stanza vnum | max globali";
        break;
    case 'O':
        hint = "Oggetto: if_flag | obj vnum | max in stanza | stanza vnum | max globali";
        break;
    case 'G':
        hint = "Give: if_flag | obj vnum | max";
        break;
    case 'E':
        hint = "Equip: if_flag | obj vnum | max | slot equip";
        break;
    case 'P':
        hint = "Put: if_flag | obj vnum | max | contenitore vnum";
        break;
    case 'D':
        hint = "Door: if_flag | stanza vnum | direzione uscita";
        break;
    case 'C':
        hint = "Command: if_flag | mob vnum | comando";
        break;
    case 'H':
        hint = "Hour: if_flag | stanza vnum | ora";
        break;
    default:
        hint = "Seleziona un comando reset";
        break;
    }
    reset_hint_->setText(hint);
}

void MainWindow::loadResetForm(const nebbie::ResetCommand& cmd) {
    const int idx = reset_command_->findData(QChar(cmd.command));
    if (idx >= 0) {
        reset_command_->setCurrentIndex(idx);
    }
    reset_if_flag_->setValue(cmd.if_flag);
    reset_arg1_->setValue(cmd.arg1);
    reset_arg2_->setValue(cmd.arg2);
    reset_arg3_->setValue(cmd.arg3);
    reset_arg4_->setValue(cmd.arg4);
    updateResetFieldHints();

    const bool editable = nebbie::is_editable_reset_command(cmd.command);
    reset_command_->setEnabled(editable);
    reset_if_flag_->setEnabled(editable);
    reset_arg1_->setEnabled(editable);
    reset_arg2_->setEnabled(editable);
    reset_arg3_->setEnabled(editable);
    reset_arg4_->setEnabled(editable);
    reset_apply_->setEnabled(editable);
    reset_remove_->setEnabled(editable);
}

nebbie::ResetCommand MainWindow::readResetForm() const {
    nebbie::ResetCommand cmd;
    cmd.command = reset_command_->currentData().toChar().toLatin1();
    cmd.if_flag = reset_if_flag_->value();
    cmd.arg1 = reset_arg1_->value();
    cmd.arg2 = reset_arg2_->value();
    cmd.arg3 = reset_arg3_->value();
    cmd.arg4 = reset_arg4_->value();
    return cmd;
}

void MainWindow::onZoneSelected() {
    const int zone_num = currentZoneNum();
    if (zone_num <= 0) {
        return;
    }
    const nebbie::Zone* zone = nebbie::find_zone(world_, zone_num);
    if (!zone) {
        return;
    }

    zone_info_->setText(QString("Zona #%1 %2\nStanze: %3-%4 | Lifespan: %5 | Reset mode: %6")
                            .arg(zone->num)
                            .arg(QString::fromStdString(zone->name))
                            .arg(zone->bottom)
                            .arg(zone->top)
                            .arg(zone->lifespan)
                            .arg(zone->reset_mode));
    refreshResetList(zone_num);
}

void MainWindow::onResetSelected() {
    const int zone_num = currentZoneNum();
    const int index = currentResetIndex();
    if (zone_num <= 0 || index < 0) {
        return;
    }
    const nebbie::Zone* zone = nebbie::find_zone(world_, zone_num);
    if (!zone || static_cast<std::size_t>(index) >= zone->commands.size()) {
        return;
    }
    loadResetForm(zone->commands[static_cast<std::size_t>(index)]);
}

void MainWindow::onResetCommandChanged() {
    updateResetFieldHints();
}

void MainWindow::addZoneReset() {
    const int zone_num = currentZoneNum();
    if (zone_num <= 0) {
        QMessageBox::information(this, "Zone", "Seleziona una zona.");
        return;
    }
    const nebbie::Zone* zone = nebbie::find_zone(world_, zone_num);
    if (!zone) {
        return;
    }

    const char command = reset_command_->currentData().toChar().toLatin1();
    const nebbie::ResetCommand cmd = nebbie::default_zone_reset(command, *zone, world_);
    if (!nebbie::add_zone_reset(world_, zone_num, cmd)) {
        QMessageBox::warning(this, "Zone", "Impossibile aggiungere il reset.");
        return;
    }

    refreshResetList(zone_num);
    reset_list_->setCurrentRow(reset_list_->count() - 1);
    markDirty();
    setStatus(QString("Aggiunto reset %1 in zona %2.").arg(QChar(command)).arg(zone_num));
}

void MainWindow::applyResetChanges() {
    const int zone_num = currentZoneNum();
    const int index = currentResetIndex();
    if (zone_num <= 0 || index < 0) {
        QMessageBox::information(this, "Zone", "Seleziona un reset modificabile.");
        return;
    }

    const nebbie::ResetCommand cmd = readResetForm();
    if (!nebbie::update_zone_reset(world_, zone_num, static_cast<std::size_t>(index), cmd)) {
        QMessageBox::warning(this, "Zone", "Impossibile aggiornare il reset.");
        return;
    }

    refreshResetList(zone_num);
    reset_list_->setCurrentRow(index);
    markDirty();
    setStatus(QString("Reset [%1] aggiornato.").arg(index));
}

void MainWindow::removeSelectedReset() {
    const int zone_num = currentZoneNum();
    const int index = currentResetIndex();
    if (zone_num <= 0 || index < 0) {
        QMessageBox::information(this, "Zone", "Seleziona un reset da eliminare.");
        return;
    }
    if (!nebbie::remove_zone_reset(world_, zone_num, static_cast<std::size_t>(index))) {
        QMessageBox::warning(this, "Zone", "Impossibile eliminare il reset.");
        return;
    }

    refreshResetList(zone_num);
    markDirty();
    setStatus(QString("Reset [%1] eliminato.").arg(index));
}

void MainWindow::moveResetUp() {
    const int zone_num = currentZoneNum();
    const int index = currentResetIndex();
    if (zone_num <= 0 || index <= 0) {
        return;
    }
    if (!nebbie::move_zone_reset(world_, zone_num, static_cast<std::size_t>(index),
                                 static_cast<std::size_t>(index - 1))) {
        return;
    }
    refreshResetList(zone_num);
    reset_list_->setCurrentRow(index - 1);
    markDirty();
}

void MainWindow::moveResetDown() {
    const int zone_num = currentZoneNum();
    const int index = currentResetIndex();
    if (zone_num <= 0 || index < 0 || index + 1 >= reset_list_->count()) {
        return;
    }
    if (!nebbie::move_zone_reset(world_, zone_num, static_cast<std::size_t>(index),
                                 static_cast<std::size_t>(index + 1))) {
        return;
    }
    refreshResetList(zone_num);
    reset_list_->setCurrentRow(index + 1);
    markDirty();
}

void MainWindow::goToResetRoom() {
    const nebbie::ResetCommand cmd = readResetForm();
    long room_vnum = 0;
    switch (cmd.command) {
    case 'M':
    case 'O':
        room_vnum = cmd.arg3;
        break;
    case 'D':
    case 'H':
        room_vnum = cmd.arg1;
        break;
    default:
        room_vnum = cmd.arg3 > 0 ? cmd.arg3 : cmd.arg1;
        break;
    }
    if (room_vnum <= 0) {
        return;
    }
    if (!world_.find_room(room_vnum)) {
        QMessageBox::information(this, "Zone",
                                 QString("La stanza #%1 non esiste in questa libreria.").arg(room_vnum));
        return;
    }
    selectRoomByVnum(room_vnum);
}

void MainWindow::goToResetEntity() {
    const nebbie::ResetCommand cmd = readResetForm();
    if (cmd.command == 'M' || cmd.command == 'C') {
        if (cmd.arg1 > 0) {
            selectMobByVnum(cmd.arg1);
        }
        return;
    }
    if (cmd.command == 'O' || cmd.command == 'G' || cmd.command == 'E' || cmd.command == 'P') {
        if (cmd.arg1 > 0) {
            selectObjectByVnum(cmd.arg1);
        }
    }
}

void MainWindow::refreshExitList(const long room_vnum) {
    exit_list_->clear();
    const nebbie::Room* room = world_.find_room(room_vnum);
    if (!room) {
        return;
    }

    for (const auto& exit : room->exits) {
        QString target_name;
        if (const nebbie::Room* target = world_.find_room(exit.to_room)) {
            target_name = QString::fromStdString(target->name);
        } else {
            target_name = "(mancante)";
        }
        const QString label = QString("%1 → #%2 %3")
                                  .arg(QString::fromUtf8(nebbie::exit_direction_name(exit.direction)))
                                  .arg(exit.to_room)
                                  .arg(target_name);
        addListItem(exit_list_, exit.direction, label);
    }
}

long MainWindow::currentRoomVnum() const {
    const auto* item = room_list_->currentItem();
    if (!item) {
        return 0;
    }
    return static_cast<long>(item->data(Qt::UserRole).toLongLong());
}

void MainWindow::selectRoomByVnum(const long vnum) {
    for (int i = 0; i < room_list_->count(); ++i) {
        if (room_list_->item(i)->data(Qt::UserRole).toLongLong() == vnum) {
            room_list_->setCurrentRow(i);
            tabs_->setCurrentIndex(0);
            return;
        }
    }
}

void MainWindow::selectMobByVnum(const long vnum) {
    for (int i = 0; i < mob_list_->count(); ++i) {
        if (mob_list_->item(i)->data(Qt::UserRole).toLongLong() == vnum) {
            mob_list_->setCurrentRow(i);
            tabs_->setCurrentIndex(1);
            return;
        }
    }
}

void MainWindow::selectObjectByVnum(const long vnum) {
    for (int i = 0; i < obj_list_->count(); ++i) {
        if (obj_list_->item(i)->data(Qt::UserRole).toLongLong() == vnum) {
            obj_list_->setCurrentRow(i);
            tabs_->setCurrentIndex(2);
            return;
        }
    }
}

void MainWindow::onRoomSearchChanged(const QString& text) {
    room_filter_ = text.trimmed();
    refreshRoomList();
}

void MainWindow::onMobSearchChanged(const QString& text) {
    mob_filter_ = text.trimmed();
    refreshMobList();
}

void MainWindow::onObjSearchChanged(const QString& text) {
    object_filter_ = text.trimmed();
    refreshObjectList();
}

void MainWindow::onRoomSelected() {
    const long vnum = currentRoomVnum();
    if (vnum <= 0) {
        return;
    }
    const nebbie::Room* room = world_.find_room(vnum);
    if (!room) {
        return;
    }
    room_name_->setText(QString::fromStdString(room->name));
    room_desc_->setPlainText(QString::fromStdString(room->description));
    room_sector_->setValue(static_cast<int>(room->sector_type));
    room_flags_->setValue(static_cast<int>(room->room_flags));
    refreshExitList(vnum);
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

void MainWindow::onExitSelected() {
    const long room_vnum = currentRoomVnum();
    const auto* item = exit_list_->currentItem();
    if (!room_vnum || !item) {
        return;
    }
    const int direction = static_cast<int>(item->data(Qt::UserRole).toLongLong());
    const nebbie::Room* room = world_.find_room(room_vnum);
    if (!room) {
        return;
    }
    const nebbie::Exit* exit = nebbie::find_room_exit(*room, direction);
    if (!exit) {
        return;
    }

    exit_direction_->setCurrentIndex(direction);
    exit_to_room_->setValue(static_cast<int>(exit->to_room));
    exit_description_->setText(QString::fromStdString(exit->description));
    exit_keyword_->setText(QString::fromStdString(exit->keyword));
    exit_info_->setValue(static_cast<int>(exit->exit_info));
    exit_key_->setValue(static_cast<int>(exit->key));
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

void MainWindow::applyExitChanges() {
    const long room_vnum = currentRoomVnum();
    if (room_vnum <= 0) {
        QMessageBox::information(this, "Uscite", "Seleziona una stanza.");
        return;
    }

    nebbie::ExitEdit edit;
    edit.direction = exit_direction_->currentData().toInt();
    edit.to_room = exit_to_room_->value();
    edit.description = exit_description_->text().toStdString();
    edit.keyword = exit_keyword_->text().toStdString();
    edit.exit_info = exit_info_->value();
    edit.key = exit_key_->value();
    edit.open_cmd = -1;

    if (!nebbie::set_room_exit(world_, room_vnum, edit)) {
        QMessageBox::warning(this, "Uscite", "Impossibile aggiornare l'uscita.");
        return;
    }

    refreshExitList(room_vnum);
    markDirty();
    setStatus(QString("Uscita %1 aggiornata per stanza %2.")
                  .arg(QString::fromUtf8(nebbie::exit_direction_name(edit.direction)))
                  .arg(room_vnum));
}

void MainWindow::removeSelectedExit() {
    const long room_vnum = currentRoomVnum();
    const auto* item = exit_list_->currentItem();
    if (room_vnum <= 0 || !item) {
        QMessageBox::information(this, "Uscite", "Seleziona un'uscita da eliminare.");
        return;
    }
    const int direction = static_cast<int>(item->data(Qt::UserRole).toLongLong());
    if (!nebbie::remove_room_exit(world_, room_vnum, direction)) {
        QMessageBox::warning(this, "Uscite", "Uscita non trovata.");
        return;
    }
    refreshExitList(room_vnum);
    markDirty();
    setStatus(QString("Uscita %1 rimossa.").arg(QString::fromUtf8(nebbie::exit_direction_name(direction))));
}

void MainWindow::goToExitTarget() {
    const auto* item = exit_list_->currentItem();
    if (!item) {
        return;
    }
    const int direction = static_cast<int>(item->data(Qt::UserRole).toLongLong());
    const nebbie::Room* room = world_.find_room(currentRoomVnum());
    if (!room) {
        return;
    }
    const nebbie::Exit* exit = nebbie::find_room_exit(*room, direction);
    if (!exit || exit->to_room <= 0) {
        return;
    }
    if (!world_.find_room(exit->to_room)) {
        QMessageBox::information(this, "Uscite",
                                 QString("La stanza #%1 non esiste in questa libreria.").arg(exit->to_room));
        return;
    }
    selectRoomByVnum(exit->to_room);
}

void MainWindow::createRoom() {
    if (lib_path_.empty()) {
        QMessageBox::information(this, "Stanze", "Apri prima una libreria.");
        return;
    }

    bool ok = false;
    const int suggested = static_cast<int>(nebbie::suggest_next_room_vnum(world_));
    const int vnum = QInputDialog::getInt(this, "Nuova stanza", "Vnum stanza:", suggested, 1, 999999, 1, &ok);
    if (!ok) {
        return;
    }

    if (!nebbie::create_room(world_, vnum)) {
        QMessageBox::warning(this, "Stanze", QString("Impossibile creare la stanza #%1 (vnum duplicato?).").arg(vnum));
        return;
    }

    refreshRoomList();
    selectRoomByVnum(vnum);
    markDirty();
    setStatus(QString("Creata stanza #%1.").arg(vnum));
}

void MainWindow::createMob() {
    if (lib_path_.empty()) {
        QMessageBox::information(this, "Mob", "Apri prima una libreria.");
        return;
    }

    bool ok = false;
    const int suggested = static_cast<int>(nebbie::suggest_next_mob_vnum(world_));
    const int vnum = QInputDialog::getInt(this, "Nuovo mob", "Vnum mobile:", suggested, 1, 99999, 1, &ok);
    if (!ok) {
        return;
    }

    if (!nebbie::create_mob(world_, vnum)) {
        QMessageBox::warning(this, "Mob", QString("Impossibile creare il mob #%1 (vnum duplicato?).").arg(vnum));
        return;
    }

    refreshMobList();
    selectMobByVnum(vnum);
    markDirty();
    setStatus(QString("Creato mob #%1.").arg(vnum));
}

void MainWindow::createObject() {
    if (lib_path_.empty()) {
        QMessageBox::information(this, "Oggetti", "Apri prima una libreria.");
        return;
    }

    bool ok = false;
    const int suggested = static_cast<int>(nebbie::suggest_next_object_vnum(world_));
    const int vnum = QInputDialog::getInt(this, "Nuovo oggetto", "Vnum oggetto:", suggested, 1, 99999, 1, &ok);
    if (!ok) {
        return;
    }

    if (!nebbie::create_object(world_, vnum)) {
        QMessageBox::warning(this, "Oggetti", QString("Impossibile creare l'oggetto #%1 (vnum duplicato?).").arg(vnum));
        return;
    }

    refreshObjectList();
    selectObjectByVnum(vnum);
    markDirty();
    setStatus(QString("Creato oggetto #%1.").arg(vnum));
}

void MainWindow::showValidation(const nebbie::ValidationReport& report) {
    validation_issues_ = report.issues;
    validation_list_->clear();

    for (std::size_t i = 0; i < validation_issues_.size(); ++i) {
        const auto& issue = validation_issues_[i];
        const char* level = issue.severity == nebbie::ValidationSeverity::error ? "ERRORE" : "AVVISO";
        const QString text = QString("[%1] %2: %3")
                                 .arg(level)
                                 .arg(QString::fromStdString(issue.category))
                                 .arg(QString::fromStdString(issue.message));
        auto* item = new QListWidgetItem(text, validation_list_);
        item->setData(Qt::UserRole, static_cast<qlonglong>(i));
        if (issue.severity == nebbie::ValidationSeverity::error) {
            item->setForeground(Qt::red);
        } else {
            item->setForeground(QColor(180, 120, 0));
        }
    }

    if (validation_list_->count() == 0) {
        validation_list_->addItem("Nessun problema rilevato.");
    } else {
        auto* summary = new QListWidgetItem(
            QString("\n%1 errori, %2 avvisi").arg(report.error_count()).arg(report.warning_count()));
        summary->setFlags(Qt::NoItemFlags);
        validation_list_->addItem(summary);
    }

    tabs_->setCurrentWidget(validation_tab_);
}

void MainWindow::navigateToIssue(const nebbie::ValidationIssue& issue) {
    switch (issue.target) {
    case nebbie::ValidationTarget::room:
        tabs_->setCurrentIndex(0);
        selectRoomByVnum(issue.target_vnum);
        break;
    case nebbie::ValidationTarget::mob:
        tabs_->setCurrentIndex(1);
        selectMobByVnum(issue.target_vnum);
        break;
    case nebbie::ValidationTarget::object:
        tabs_->setCurrentIndex(2);
        selectObjectByVnum(issue.target_vnum);
        break;
    case nebbie::ValidationTarget::zone:
        if (zone_tab_) {
            tabs_->setCurrentWidget(zone_tab_);
        }
        selectZoneByNum(issue.zone_num);
        if (issue.reset_index >= 0 && reset_list_ && issue.reset_index < reset_list_->count()) {
            reset_list_->setCurrentRow(issue.reset_index);
        }
        break;
    case nebbie::ValidationTarget::shop:
        setStatus(QString("Shop #%1 — usa la CLI per i dettagli shop.").arg(issue.target_vnum));
        break;
    default:
        break;
    }
}

void MainWindow::onValidationIssueActivated(QListWidgetItem* item) {
    if (!item) {
        return;
    }
    const int index = static_cast<int>(item->data(Qt::UserRole).toLongLong());
    if (index < 0 || index >= static_cast<int>(validation_issues_.size())) {
        return;
    }
    navigateToIssue(validation_issues_[static_cast<std::size_t>(index)]);
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
        nebbie::save_lib_with_backup(world_, context_, lib_path_);
        markClean();
        last_version_time_ = std::chrono::system_clock::now();
        setStatus("Libreria salvata (backup creato in .nebbie/versions).");
        QMessageBox::information(this, "Salva", "Salvataggio completato. Backup pre-salvataggio creato.");
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
        nebbie::save_lib_with_backup(world_, context_, lib_path_);
        markClean();
        last_version_time_ = std::chrono::system_clock::now();
        setStatus("Libreria salvata (forzato, backup creato).");
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, "Errore", QString::fromUtf8(ex.what()));
    }
}

void MainWindow::onAutosaveTick() {
    if (!dirty_ || lib_path_.empty()) {
        return;
    }

    try {
        const auto result = nebbie::run_autosave(world_, context_, lib_path_, session_config_, last_version_time_);
        if (result.version_created) {
            last_version_time_ = std::chrono::system_clock::now();
        }
        const QString time = QDateTime::currentDateTime().toString("HH:mm:ss");
        if (result.version_created) {
            setStatus(QString("Autosalvataggio + versione %1 (%2)")
                          .arg(QString::fromStdString(result.version_id))
                          .arg(time));
        } else {
            setStatus(QString("Autosalvataggio workspace (%1)").arg(time));
        }
    } catch (const std::exception& ex) {
        setStatus(QString("Autosalvataggio fallito: %1").arg(QString::fromUtf8(ex.what())));
    }
}

void MainWindow::restoreFromWorkspace() {
    if (lib_path_.empty()) {
        QMessageBox::information(this, "Cronologia", "Apri prima una libreria.");
        return;
    }

    const auto workspace = nebbie::workspace_dir(lib_path_);
    std::error_code ec;
    if (!std::filesystem::exists(workspace, ec)) {
        QMessageBox::information(this, "Cronologia", "Nessun autosalvataggio workspace trovato.");
        return;
    }

    const auto answer = QMessageBox::question(
        this, "Ripristina autosalvataggio",
        "Ripristinare l'ultimo autosalvataggio workspace? Le modifiche correnti non salvate andranno perse.",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }

    try {
        nebbie::restore_snapshot(world_, context_, workspace);
        markDirty();
        refreshRoomList();
        refreshMobList();
        refreshObjectList();
        refreshZoneList();
        setStatus("Ripristinato autosalvataggio workspace.");
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, "Errore", QString::fromUtf8(ex.what()));
    }
}

void MainWindow::restoreVersion() {
    if (lib_path_.empty()) {
        QMessageBox::information(this, "Cronologia", "Apri prima una libreria.");
        return;
    }

    const auto versions = nebbie::list_versions(lib_path_);
    if (versions.empty()) {
        QMessageBox::information(this, "Cronologia", "Nessuna versione salvata in .nebbie/versions.");
        return;
    }

    QStringList labels;
    for (const auto& version : versions) {
        labels << QString::fromStdString(version.id + " [" + version.label + "]");
    }

    bool ok = false;
    const QString chosen = QInputDialog::getItem(
        this, "Ripristina versione", "Seleziona versione:", labels, 0, false, &ok);
    if (!ok || chosen.isEmpty()) {
        return;
    }

    const int index = labels.indexOf(chosen);
    if (index < 0) {
        return;
    }

    const auto answer = QMessageBox::question(
        this, "Ripristina versione",
        "Ripristinare la versione selezionata? Le modifiche correnti non salvate andranno perse.",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }

    try {
        const auto snapshot = nebbie::versions_dir(lib_path_) / versions[static_cast<std::size_t>(index)].id;
        nebbie::restore_snapshot(world_, context_, snapshot);
        markDirty();
        refreshRoomList();
        refreshMobList();
        refreshObjectList();
        refreshZoneList();
        setStatus(QString("Ripristinata versione %1.").arg(chosen));
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

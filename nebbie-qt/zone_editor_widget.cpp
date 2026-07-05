#include "zone_editor_widget.hpp"

#include "nebbie/edit.hpp"
#include "nebbie/zone_catalog.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

namespace {

void configureLineField(QLineEdit* field) {
    field->setMinimumWidth(420);
    field->setMinimumHeight(30);
    field->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

QLabel* makeLegend(const QString& text, QWidget* parent) {
    auto* label = new QLabel(text, parent);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}

void fillIntCombo(QComboBox* combo, const std::vector<std::pair<int, std::string>>& choices) {
    combo->clear();
    for (const auto& [value, label] : choices) {
        combo->addItem(QString::fromStdString(label), value);
    }
}

void fillCharCombo(QComboBox* combo, const std::vector<std::pair<char, std::string>>& choices) {
    combo->clear();
    for (const auto& [value, label] : choices) {
        combo->addItem(QString::fromStdString(label), QChar(value));
    }
}

int comboIntValue(QComboBox* combo) {
    return combo->currentData().toInt();
}

} // namespace

void ZoneEditorWidget::setComboIntValue(QComboBox* combo, const int value) const {
    const int index = combo->findData(value);
    combo->setCurrentIndex(index >= 0 ? index : 0);
}

ZoneEditorWidget::ZoneEditorWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    auto* tabs = new QTabWidget;

    auto* info_tab = new QWidget;
    auto* info_layout = new QVBoxLayout(info_tab);
    info_layout->addWidget(makeLegend(
        "Zone header in myst.zon: #<num>, name~, then top lifespan reset_mode. "
        "bottom vnum is derived from the previous zone top + 1. "
        "lifespan is real-time minutes between resets. reset_mode: 0=never, 1=when deserted, 2=normal.",
        info_tab));
    auto* info_form = new QFormLayout;
    zone_num_label_ = new QLabel("-");
    name_ = new QLineEdit;
    configureLineField(name_);
    bottom_label_ = new QLabel("-");
    top_ = new QSpinBox;
    top_->setRange(0, 999999);
    lifespan_ = new QSpinBox;
    lifespan_->setRange(0, 9999);
    lifespan_->setToolTip("Minutes between zone resets.");
    reset_mode_ = new QComboBox;
    fillIntCombo(reset_mode_, nebbie::zone_reset_mode_choices());
    info_form->addRow("Zone #:", zone_num_label_);
    info_form->addRow("Name:", name_);
    info_form->addRow("bottom vnum:", bottom_label_);
    info_form->addRow("top vnum:", top_);
    info_form->addRow("lifespan:", lifespan_);
    info_form->addRow("reset_mode:", reset_mode_);
    info_layout->addLayout(info_form);
    info_layout->addStretch();
    tabs->addTab(info_tab, "Zone info");

    auto* reset_tab = new QWidget;
    auto* reset_layout = new QVBoxLayout(reset_tab);
    reset_layout->addWidget(makeLegend(
        "Reset commands run in order when the zone resets. if_flag: 0=always, 1=only if the previous "
        "command succeeded. G/E/P depend on the last M or O command. Comments (*) and separators (;) "
        "are shown read-only. Table ends with S in myst.zon.",
        reset_tab));
    reset_list_ = new QListWidget;
    reset_list_->setMinimumHeight(140);
    reset_layout->addWidget(reset_list_);

    reset_command_ = new QComboBox;
    fillCharCombo(reset_command_, nebbie::zone_reset_command_choices());
    reset_legend_ = new QLabel;
    reset_legend_->setWordWrap(true);
    if_flag_label_ = new QLabel;
    arg1_label_ = new QLabel;
    arg2_label_ = new QLabel;
    arg3_label_ = new QLabel;
    arg4_label_ = new QLabel;
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

    equip_panel_ = new QWidget(reset_tab);
    auto* equip_form = new QFormLayout(equip_panel_);
    equip_position_ = new QComboBox;
    fillIntCombo(equip_position_, nebbie::zone_equip_position_choices());
    equip_form->addRow("equip position:", equip_position_);
    door_state_panel_ = new QWidget(reset_tab);
    auto* door_form = new QFormLayout(door_state_panel_);
    door_state_ = new QComboBox;
    fillIntCombo(door_state_, nebbie::zone_door_state_choices());
    door_form->addRow("door state:", door_state_);

    auto* reset_form = new QFormLayout;
    reset_form->addRow("Command:", reset_command_);
    reset_form->addRow(reset_legend_);
    reset_form->addRow(if_flag_label_, reset_if_flag_);
    reset_form->addRow(arg1_label_, reset_arg1_);
    reset_form->addRow(arg2_label_, reset_arg2_);
    reset_form->addRow(arg3_label_, reset_arg3_);
    reset_form->addRow(arg4_label_, reset_arg4_);
    reset_form->addRow(equip_panel_);
    reset_form->addRow(door_state_panel_);
    reset_layout->addLayout(reset_form);

    auto* reset_buttons = new QHBoxLayout;
    auto* reset_add = new QPushButton("New reset");
    auto* reset_apply = new QPushButton("Apply");
    auto* reset_remove = new QPushButton("Remove");
    auto* reset_up = new QPushButton("Up");
    auto* reset_down = new QPushButton("Down");
    auto* reset_goto_room = new QPushButton("Go to room");
    auto* reset_goto_entity = new QPushButton("Go to entity");
    reset_buttons->addWidget(reset_add);
    reset_buttons->addWidget(reset_apply);
    reset_buttons->addWidget(reset_remove);
    reset_buttons->addWidget(reset_up);
    reset_buttons->addWidget(reset_down);
    reset_buttons->addWidget(reset_goto_room);
    reset_buttons->addWidget(reset_goto_entity);
    reset_buttons->addStretch();
    reset_layout->addLayout(reset_buttons);
    tabs->addTab(new QScrollArea, "Reset (myst.zon)");
    {
        auto* scroll = qobject_cast<QScrollArea*>(tabs->widget(1));
        scroll->setWidgetResizable(true);
        scroll->setWidget(reset_tab);
    }

    root->addWidget(tabs);

    connect(reset_list_, &QListWidget::currentRowChanged, this, [this](int) { onResetSelected(); });
    connect(reset_command_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &ZoneEditorWidget::onResetCommandChanged);
    connect(reset_add, &QPushButton::clicked, this, &ZoneEditorWidget::addReset);
    connect(reset_apply, &QPushButton::clicked, this, &ZoneEditorWidget::applyReset);
    connect(reset_remove, &QPushButton::clicked, this, &ZoneEditorWidget::removeReset);
    connect(reset_up, &QPushButton::clicked, this, &ZoneEditorWidget::moveResetUp);
    connect(reset_down, &QPushButton::clicked, this, &ZoneEditorWidget::moveResetDown);
    connect(reset_goto_room, &QPushButton::clicked, this, &ZoneEditorWidget::gotoResetRoom);
    connect(reset_goto_entity, &QPushButton::clicked, this, &ZoneEditorWidget::gotoResetEntity);
    connect(equip_position_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        if (reset_command_->currentData().toChar() == QLatin1Char('E')) {
            reset_arg3_->setValue(comboIntValue(equip_position_));
        }
    });
    connect(door_state_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        if (reset_command_->currentData().toChar() == QLatin1Char('D')) {
            reset_arg3_->setValue(comboIntValue(door_state_));
        }
    });

    equip_panel_->setVisible(false);
    door_state_panel_->setVisible(false);
    onResetCommandChanged();
}

void ZoneEditorWidget::selectResetIndex(const int index) {
    if (index >= 0 && index < reset_list_->count()) {
        reset_list_->setCurrentRow(index);
    }
}

void ZoneEditorWidget::setWorld(nebbie::World* world) {
    world_ = world;
}

void ZoneEditorWidget::loadFromZone(const nebbie::Zone& zone) {
    zone_num_ = zone.num;
    zone_num_label_->setText(QString::number(zone.num));
    name_->setText(QString::fromStdString(zone.name));
    bottom_label_->setText(QString("%1").arg(zone.bottom));
    top_->setValue(zone.top);
    lifespan_->setValue(zone.lifespan);
    setComboIntValue(reset_mode_, zone.reset_mode);
    refreshResetList();
}

void ZoneEditorWidget::saveZoneInfoTo(nebbie::Zone& zone) const {
    zone.name = name_->text().toStdString();
    zone.top = top_->value();
    zone.lifespan = lifespan_->value();
    zone.reset_mode = comboIntValue(reset_mode_);
}

void ZoneEditorWidget::refreshResetList() {
    reset_list_->clear();
    if (!world_ || zone_num_ <= 0) {
        return;
    }
    const nebbie::Zone* zone = nebbie::find_zone(*world_, zone_num_);
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

void ZoneEditorWidget::updateResetFieldLabels() {
    const char command = reset_command_->currentData().toChar().toLatin1();
    reset_legend_->setText(QString::fromStdString(nebbie::zone_reset_command_legend(command)));
    const nebbie::ResetArgLabels labels = nebbie::zone_reset_arg_labels(command);
    if_flag_label_->setText(QString::fromStdString(labels.if_flag));
    arg1_label_->setText(QString::fromStdString(labels.arg1));
    arg2_label_->setText(QString::fromStdString(labels.arg2));
    arg3_label_->setText(QString::fromStdString(labels.arg3));
    arg4_label_->setText(QString::fromStdString(labels.arg4));

    const bool is_equip = command == 'E';
    const bool is_door = command == 'D';
    equip_panel_->setVisible(is_equip);
    door_state_panel_->setVisible(is_door);
    reset_arg3_->setVisible(!is_equip && !is_door);
    arg3_label_->setVisible(!is_equip && !is_door);

    const bool use_arg4 = command == 'M' || command == 'O';
    reset_arg4_->setVisible(use_arg4);
    arg4_label_->setVisible(use_arg4);
}

int ZoneEditorWidget::currentResetIndex() const {
    const auto* item = reset_list_->currentItem();
    if (!item) {
        return -1;
    }
    return static_cast<int>(item->data(Qt::UserRole).toLongLong());
}

void ZoneEditorWidget::loadResetForm(const nebbie::ResetCommand& cmd) {
    const int idx = reset_command_->findData(QChar(cmd.command));
    if (idx >= 0) {
        reset_command_->setCurrentIndex(idx);
    }
    reset_if_flag_->setValue(cmd.if_flag);
    reset_arg1_->setValue(cmd.arg1);
    reset_arg2_->setValue(cmd.arg2);
    reset_arg3_->setValue(cmd.arg3);
    reset_arg4_->setValue(cmd.arg4);
    if (cmd.command == 'E') {
        setComboIntValue(equip_position_, cmd.arg3);
    }
    if (cmd.command == 'D') {
        setComboIntValue(door_state_, cmd.arg3);
    }
    updateResetFieldLabels();

    const bool editable = nebbie::is_editable_reset_command(cmd.command);
    reset_command_->setEnabled(editable);
    reset_if_flag_->setEnabled(editable);
    reset_arg1_->setEnabled(editable);
    reset_arg2_->setEnabled(editable);
    reset_arg3_->setEnabled(editable);
    reset_arg4_->setEnabled(editable);
    equip_position_->setEnabled(editable);
    door_state_->setEnabled(editable);
}

nebbie::ResetCommand ZoneEditorWidget::readResetForm() const {
    nebbie::ResetCommand cmd;
    cmd.command = reset_command_->currentData().toChar().toLatin1();
    cmd.if_flag = reset_if_flag_->value();
    cmd.arg1 = reset_arg1_->value();
    cmd.arg2 = reset_arg2_->value();
    cmd.arg3 = reset_arg3_->value();
    cmd.arg4 = reset_arg4_->value();
    if (cmd.command == 'E') {
        cmd.arg3 = comboIntValue(equip_position_);
    }
    if (cmd.command == 'D') {
        cmd.arg3 = comboIntValue(door_state_);
    }
    return cmd;
}

void ZoneEditorWidget::onResetSelected() {
    if (!world_ || zone_num_ <= 0) {
        return;
    }
    const int index = currentResetIndex();
    if (index < 0) {
        return;
    }
    const nebbie::Zone* zone = nebbie::find_zone(*world_, zone_num_);
    if (!zone || static_cast<std::size_t>(index) >= zone->commands.size()) {
        return;
    }
    loadResetForm(zone->commands[static_cast<std::size_t>(index)]);
}

void ZoneEditorWidget::onResetCommandChanged() {
    updateResetFieldLabels();
}

void ZoneEditorWidget::addReset() {
    if (!world_ || zone_num_ <= 0) {
        QMessageBox::information(this, "Zone", "Select a zone.");
        return;
    }
    const nebbie::Zone* zone = nebbie::find_zone(*world_, zone_num_);
    if (!zone) {
        return;
    }

    const char command = reset_command_->currentData().toChar().toLatin1();
    const nebbie::ResetCommand cmd = nebbie::default_zone_reset(command, *zone, *world_);
    if (!nebbie::add_zone_reset(*world_, zone_num_, cmd)) {
        QMessageBox::warning(this, "Zone", "Unable to add reset.");
        return;
    }

    refreshResetList();
    reset_list_->setCurrentRow(reset_list_->count() - 1);
    emit zoneModified();
}

void ZoneEditorWidget::applyReset() {
    if (!world_ || zone_num_ <= 0) {
        return;
    }
    const int index = currentResetIndex();
    if (index < 0) {
        QMessageBox::information(this, "Zone", "Select an editable reset.");
        return;
    }

    const nebbie::ResetCommand cmd = readResetForm();
    if (!nebbie::update_zone_reset(*world_, zone_num_, static_cast<std::size_t>(index), cmd)) {
        QMessageBox::warning(this, "Zone", "Unable to update reset.");
        return;
    }

    refreshResetList();
    reset_list_->setCurrentRow(index);
    emit zoneModified();
}

void ZoneEditorWidget::removeReset() {
    if (!world_ || zone_num_ <= 0) {
        return;
    }
    const int index = currentResetIndex();
    if (index < 0) {
        QMessageBox::information(this, "Zone", "Select a reset to remove.");
        return;
    }
    if (!nebbie::remove_zone_reset(*world_, zone_num_, static_cast<std::size_t>(index))) {
        QMessageBox::warning(this, "Zone", "Unable to remove reset.");
        return;
    }

    refreshResetList();
    emit zoneModified();
}

void ZoneEditorWidget::moveResetUp() {
    if (!world_ || zone_num_ <= 0) {
        return;
    }
    const int index = currentResetIndex();
    if (index <= 0) {
        return;
    }
    if (!nebbie::move_zone_reset(*world_, zone_num_, static_cast<std::size_t>(index),
                                 static_cast<std::size_t>(index - 1))) {
        return;
    }
    refreshResetList();
    reset_list_->setCurrentRow(index - 1);
    emit zoneModified();
}

void ZoneEditorWidget::moveResetDown() {
    if (!world_ || zone_num_ <= 0) {
        return;
    }
    const int index = currentResetIndex();
    if (index < 0 || index + 1 >= reset_list_->count()) {
        return;
    }
    if (!nebbie::move_zone_reset(*world_, zone_num_, static_cast<std::size_t>(index),
                                 static_cast<std::size_t>(index + 1))) {
        return;
    }
    refreshResetList();
    reset_list_->setCurrentRow(index + 1);
    emit zoneModified();
}

void ZoneEditorWidget::gotoResetRoom() {
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
    if (room_vnum > 0) {
        emit gotoRoomRequested(room_vnum);
    }
}

void ZoneEditorWidget::gotoResetEntity() {
    const nebbie::ResetCommand cmd = readResetForm();
    if (cmd.command == 'M' || cmd.command == 'C') {
        if (cmd.arg1 > 0) {
            emit gotoMobRequested(cmd.arg1);
        }
        return;
    }
    if (cmd.command == 'O' || cmd.command == 'G' || cmd.command == 'E' || cmd.command == 'P') {
        if (cmd.arg1 > 0) {
            emit gotoObjectRequested(cmd.arg1);
        }
    }
}

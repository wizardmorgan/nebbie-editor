#include "mob_editor_widget.hpp"

#include "flag_group_widget.hpp"
#include "nebbie/mob_catalog.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {

QWidget* makeDiceRow(QSpinBox*& number, QSpinBox*& size, QSpinBox*& plus, QWidget* parent) {
    auto* row = new QWidget(parent);
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);

    number = new QSpinBox(row);
    number->setRange(0, 9999);
    size = new QSpinBox(row);
    size->setRange(1, 9999);
    plus = new QSpinBox(row);
    plus->setRange(-999999, 999999);

    layout->addWidget(new QLabel("num", row));
    layout->addWidget(number);
    layout->addWidget(new QLabel("d", row));
    layout->addWidget(size);
    layout->addWidget(new QLabel("plus", row));
    layout->addWidget(plus);
    layout->addStretch();
    return row;
}

void loadDice(const std::string& text, QSpinBox* number, QSpinBox* size, QSpinBox* plus) {
    try {
        const nebbie::DiceValues dice = nebbie::parse_dice(text);
        number->setValue(dice.number);
        size->setValue(dice.size);
        plus->setValue(dice.plus);
    } catch (const std::exception&) {
        number->setValue(1);
        size->setValue(1);
        plus->setValue(0);
    }
}

std::string saveDice(QSpinBox* number, QSpinBox* size, QSpinBox* plus) {
    nebbie::DiceValues dice;
    dice.number = number->value();
    dice.size = size->value();
    dice.plus = plus->value();
    return nebbie::format_dice(dice);
}

void fillCombo(QComboBox* combo, const std::vector<std::pair<int, std::string>>& choices) {
    combo->clear();
    for (const auto& [value, label] : choices) {
        combo->addItem(QString::fromStdString(label), value);
    }
}

void fillTypeCombo(QComboBox* combo) {
    combo->clear();
    for (const auto& [value, label] : nebbie::mob_type_choices()) {
        combo->addItem(QString::fromStdString(label), QChar(value));
    }
}

int comboIntValue(QComboBox* combo) {
    return combo->currentData().toInt();
}

char comboTypeValue(QComboBox* combo) {
    return combo->currentData().toChar().toLatin1();
}

void setComboIntValue(QComboBox* combo, int value) {
    const int index = combo->findData(value);
    combo->setCurrentIndex(index >= 0 ? index : 0);
}

void setComboTypeValue(QComboBox* combo, char mobtype) {
    const int index = combo->findData(QChar(mobtype));
    combo->setCurrentIndex(index >= 0 ? index : 0);
}

} // namespace

MobEditorWidget::MobEditorWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    auto* tabs = new QTabWidget;

    auto* text_tab = new QWidget;
    auto* text_form = new QFormLayout(text_tab);
    name_ = new QLineEdit;
    short_descr_ = new QLineEdit;
    long_descr_ = new QTextEdit;
    long_descr_->setMinimumHeight(70);
    description_ = new QTextEdit;
    description_->setMinimumHeight(90);
    text_form->addRow("Keywords (name~):", name_);
    text_form->addRow("Nome breve:", short_descr_);
    text_form->addRow("Descrizione lunga:", long_descr_);
    text_form->addRow("Descrizione (look):", description_);
    tabs->addTab(text_tab, "Testo");

    auto* combat_tab = new QWidget;
    auto* combat_form = new QFormLayout(combat_tab);
    mobtype_ = new QComboBox;
    fillTypeCombo(mobtype_);
    mult_att_ = new QSpinBox;
    mult_att_->setRange(1, 20);
    level_ = new QSpinBox;
    level_->setRange(0, 200);
    hitroll_ = new QSpinBox;
    hitroll_->setRange(-50, 50);
    ac_ = new QSpinBox;
    ac_->setRange(-200, 200);
    hit_bonus_ = new QSpinBox;
    hit_bonus_->setRange(0, 999999);
    hit_dice_row_ = makeDiceRow(hit_num_, hit_size_, hit_plus_, combat_tab);
    hit_bonus_row_ = new QWidget(combat_tab);
    auto* hit_bonus_layout = new QHBoxLayout(hit_bonus_row_);
    hit_bonus_layout->setContentsMargins(0, 0, 0, 0);
    hit_bonus_layout->addWidget(hit_bonus_);
    hit_bonus_layout->addStretch();
    auto* dam_row = makeDiceRow(dam_num_, dam_size_, dam_plus_, combat_tab);

    combat_form->addRow("Tipo mob:", mobtype_);
    combat_form->addRow("Attacchi (mult_att):", mult_att_);
    combat_form->addRow("Livello:", level_);
    combat_form->addRow("Hitroll:", hitroll_);
    combat_form->addRow("Armor class:", ac_);
    combat_form->addRow("Hit dice (tipo S):", hit_dice_row_);
    combat_form->addRow("Hit bonus (tipo A/N/B/L):", hit_bonus_row_);
    combat_form->addRow("Damage dice:", dam_row);
    tabs->addTab(combat_tab, "Combattimento");

    auto* economy_tab = new QWidget;
    auto* economy_form = new QFormLayout(economy_tab);
    alignment_ = new QSpinBox;
    alignment_->setRange(-1000, 1000);
    gold_ = new QSpinBox;
    gold_->setRange(0, 2000000000);
    exp_ = new QSpinBox;
    exp_->setRange(0, 2000000000);
    extended_gold_ = new QCheckBox("Formato esteso (-1 gold exp race)");
    race_ = new QComboBox;
    fillCombo(race_, nebbie::mob_race_choices());
    race_row_ = new QWidget(economy_tab);
    auto* race_layout = new QHBoxLayout(race_row_);
    race_layout->setContentsMargins(0, 0, 0, 0);
    race_layout->addWidget(race_);
    race_layout->addStretch();
    economy_form->addRow("Allineamento:", alignment_);
    economy_form->addRow("Oro:", gold_);
    economy_form->addRow("Esperienza:", exp_);
    economy_form->addRow(extended_gold_);
    economy_form->addRow("Razza:", race_row_);
    tabs->addTab(economy_tab, "Economia");

    auto* behavior_tab = new QWidget;
    auto* behavior_layout = new QVBoxLayout(behavior_tab);
    act_flags_ = new FlagGroupWidget(nebbie::mob_act_flags(), behavior_tab);
    affected_flags_ = new FlagGroupWidget(nebbie::mob_affected_flags(), behavior_tab);
    behavior_layout->addWidget(new QLabel("Action flags (act)"));
    behavior_layout->addWidget(act_flags_);
    behavior_layout->addWidget(new QLabel("Affect flags (affected_by)"));
    behavior_layout->addWidget(affected_flags_);
    tabs->addTab(new QScrollArea, "Comportamento");
    {
        auto* scroll = qobject_cast<QScrollArea*>(tabs->widget(3));
        scroll->setWidgetResizable(true);
        scroll->setWidget(behavior_tab);
    }

    auto* resist_tab = new QWidget;
    auto* resist_layout = new QVBoxLayout(resist_tab);
    position_ = new QComboBox;
    default_pos_ = new QComboBox;
    fillCombo(position_, nebbie::mob_position_choices());
    fillCombo(default_pos_, nebbie::mob_position_choices());
    sex_ = new QComboBox;
    fillCombo(sex_, nebbie::mob_sex_choices());
    extended_sex_ = new QCheckBox("Riga estesa con immune / meta-immune / suscept");
    immunity_panel_ = new QWidget(resist_tab);
    auto* immunity_layout = new QVBoxLayout(immunity_panel_);
    immune_flags_ = new FlagGroupWidget(nebbie::mob_immunity_flags(), immunity_panel_);
    meta_immune_flags_ = new FlagGroupWidget(nebbie::mob_immunity_flags(), immunity_panel_);
    susceptible_flags_ = new FlagGroupWidget(nebbie::mob_immunity_flags(), immunity_panel_);
    immunity_layout->addWidget(new QLabel("Resistenze (immune)"));
    immunity_layout->addWidget(immune_flags_);
    immunity_layout->addWidget(new QLabel("Immunita' (M_immune)"));
    immunity_layout->addWidget(meta_immune_flags_);
    immunity_layout->addWidget(new QLabel("Susceptibilita'"));
    immunity_layout->addWidget(susceptible_flags_);

    auto* resist_form = new QFormLayout;
    resist_form->addRow("Posizione:", position_);
    resist_form->addRow("Posizione default:", default_pos_);
    resist_form->addRow("Sesso:", sex_);
    resist_form->addRow(extended_sex_);
    resist_layout->addLayout(resist_form);
    resist_layout->addWidget(immunity_panel_);
    tabs->addTab(new QScrollArea, "Posizione / Resistenze");
    {
        auto* scroll = qobject_cast<QScrollArea*>(tabs->widget(4));
        scroll->setWidgetResizable(true);
        scroll->setWidget(resist_tab);
    }

    auto* sound_tab = new QWidget;
    auto* sound_form = new QFormLayout(sound_tab);
    sounds_ = new QLineEdit;
    distant_sounds_ = new QLineEdit;
    extra_sounds_ = new QTextEdit;
    extra_sounds_->setPlaceholderText("Una stringa suono per riga (dopo suoni locali e distanti)");
    extra_sounds_->setMinimumHeight(80);
    sound_form->addRow("Suoni locali:", sounds_);
    sound_form->addRow("Suoni distanti:", distant_sounds_);
    sound_form->addRow("Suoni extra:", extra_sounds_);
    sounds_panel_ = sound_tab;
    tabs->addTab(sound_tab, "Suoni");

    root->addWidget(tabs);

    connect(mobtype_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int) { updateTypeDependentFields(); });
    connect(extended_gold_, &QCheckBox::toggled, race_row_, &QWidget::setVisible);
    connect(extended_sex_, &QCheckBox::toggled, immunity_panel_, &QWidget::setVisible);

    race_row_->setVisible(false);
    immunity_panel_->setVisible(false);
    updateTypeDependentFields();
}

void MobEditorWidget::updateTypeDependentFields() {
    const char mobtype = comboTypeValue(mobtype_);
    const bool uses_hit_dice = nebbie::mob_uses_hit_dice(mobtype);
    hit_dice_row_->setVisible(uses_hit_dice);
    hit_bonus_row_->setVisible(!uses_hit_dice);
    mult_att_->setEnabled(nebbie::mob_type_uses_mult_att(mobtype));
    sounds_panel_->setEnabled(nebbie::mob_type_uses_sounds(mobtype) || !sounds_->text().isEmpty()
                              || !distant_sounds_->text().isEmpty());
}

void MobEditorWidget::loadFromMobile(const nebbie::Mobile& mob) {
    name_->setText(QString::fromStdString(mob.name));
    short_descr_->setText(QString::fromStdString(mob.short_descr));
    long_descr_->setPlainText(QString::fromStdString(mob.long_descr));
    description_->setPlainText(QString::fromStdString(mob.description));

    setComboTypeValue(mobtype_, mob.mobtype);
    mult_att_->setValue(mob.mult_att);
    level_->setValue(mob.level);
    hitroll_->setValue(mob.hitroll);
    ac_->setValue(mob.ac);
    hit_bonus_->setValue(mob.hit_bonus);
    loadDice(mob.hit_dice, hit_num_, hit_size_, hit_plus_);
    loadDice(mob.dam_dice, dam_num_, dam_size_, dam_plus_);

    alignment_->setValue(static_cast<int>(mob.alignment));
    gold_->setValue(static_cast<int>(mob.gold));
    exp_->setValue(static_cast<int>(mob.exp));
    extended_gold_->setChecked(mob.extended_gold);
    setComboIntValue(race_, static_cast<int>(mob.race));
    race_row_->setVisible(mob.extended_gold);

    setComboIntValue(position_, mob.position);
    setComboIntValue(default_pos_, mob.default_pos);
    setComboIntValue(sex_, mob.sex);
    extended_sex_->setChecked(mob.extended_sex);
    immunity_panel_->setVisible(mob.extended_sex);
    immune_flags_->setValue(mob.immune);
    meta_immune_flags_->setValue(mob.meta_immune);
    susceptible_flags_->setValue(mob.susceptible);

    act_flags_->setValue(mob.act);
    affected_flags_->setValue(mob.affected_by);

    sounds_->setText(QString::fromStdString(mob.sounds));
    distant_sounds_->setText(QString::fromStdString(mob.distant_sounds));
    {
        QStringList lines;
        for (const auto& line : mob.extra_sound_strings) {
            lines.push_back(QString::fromStdString(line));
        }
        extra_sounds_->setPlainText(lines.join('\n'));
    }

    updateTypeDependentFields();
}

void MobEditorWidget::saveToMobile(nebbie::Mobile& mob) const {
    mob.name = name_->text().toStdString();
    mob.short_descr = short_descr_->text().toStdString();
    mob.long_descr = long_descr_->toPlainText().toStdString();
    mob.description = description_->toPlainText().toStdString();

    mob.mobtype = comboTypeValue(mobtype_);
    mob.mult_att = mult_att_->value();
    mob.level = level_->value();
    mob.hitroll = hitroll_->value();
    mob.ac = ac_->value();
    mob.hit_bonus = hit_bonus_->value();
    if (nebbie::mob_uses_hit_dice(mob.mobtype)) {
        mob.hit_dice = saveDice(hit_num_, hit_size_, hit_plus_);
        mob.hit_bonus = 0;
    } else {
        mob.hit_dice.clear();
    }
    mob.dam_dice = saveDice(dam_num_, dam_size_, dam_plus_);

    mob.alignment = alignment_->value();
    mob.gold = gold_->value();
    mob.exp = exp_->value();
    mob.extended_gold = extended_gold_->isChecked();
    mob.race = mob.extended_gold ? comboIntValue(race_) : 0;

    mob.position = comboIntValue(position_);
    mob.default_pos = comboIntValue(default_pos_);
    mob.sex = comboIntValue(sex_);
    mob.extended_sex = extended_sex_->isChecked();
    if (mob.extended_sex) {
        mob.immune = immune_flags_->value();
        mob.meta_immune = meta_immune_flags_->value();
        mob.susceptible = susceptible_flags_->value();
    } else {
        mob.immune = 0;
        mob.meta_immune = 0;
        mob.susceptible = 0;
    }

    mob.act = act_flags_->value();
    mob.affected_by = affected_flags_->value();

    mob.sounds = sounds_->text().toStdString();
    mob.distant_sounds = distant_sounds_->text().toStdString();
    mob.extra_sound_strings.clear();
    for (const QString& line : extra_sounds_->toPlainText().split('\n')) {
        const std::string value = line.trimmed().toStdString();
        if (!value.empty()) {
            mob.extra_sound_strings.push_back(value);
        }
    }
}

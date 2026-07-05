#pragma once

#include "nebbie/mob_catalog.hpp"

#include <QWidget>

class QCheckBox;
class QGridLayout;

class FlagGroupWidget : public QWidget {
    Q_OBJECT

public:
    explicit FlagGroupWidget(const std::vector<nebbie::MobFlagDef>& defs, QWidget* parent = nullptr);

    void setValue(long flags);
    long value() const;

private:
    std::vector<nebbie::MobFlagDef> defs_;
    std::vector<QCheckBox*> boxes_;
};

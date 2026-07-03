#pragma once

#include "nebbie/lib_context.hpp"
#include "nebbie/validate.hpp"
#include "nebbie/world.hpp"

#include <QMainWindow>

#include <filesystem>

class QTabWidget;
class QListWidget;
class QLineEdit;
class QTextEdit;
class QSpinBox;
class QLabel;
class QPlainTextEdit;
class QCloseEvent;
class QWidget;
class QComboBox;
class QPushButton;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    void openLibPath(const QString& path);

public slots:
    void openLib();
    void saveLib();
    void saveLibForce();
    void validateLib();

private slots:
    void onRoomSelected();
    void onMobSelected();
    void onObjSelected();
    void applyRoomChanges();
    void applyMobChanges();
    void applyObjChanges();
    void createRoom();
    void createMob();
    void createObject();
    void onRoomSearchChanged(const QString& text);
    void onMobSearchChanged(const QString& text);
    void onObjSearchChanged(const QString& text);
    void onExitSelected();
    void applyExitChanges();
    void removeSelectedExit();
    void goToExitTarget();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUi();
    void setupMenus();
    void loadLib(const std::filesystem::path& path);
    void refreshRoomList();
    void refreshMobList();
    void refreshObjectList();
    void refreshExitList(long room_vnum);
    void selectRoomByVnum(long vnum);
    void selectMobByVnum(long vnum);
    void selectObjectByVnum(long vnum);
    long currentRoomVnum() const;
    void setStatus(const QString& message);
    void showValidation(const nebbie::ValidationReport& report);
    bool confirmSaveIfDirty();
    void markDirty();
    void markClean();

    nebbie::World world_;
    nebbie::LibContext context_;
    std::filesystem::path lib_path_;
    bool dirty_ = false;

    QString room_filter_;
    QString mob_filter_;
    QString object_filter_;

    QLabel* lib_label_ = nullptr;
    QTabWidget* tabs_ = nullptr;

    QListWidget* room_list_ = nullptr;
    QLineEdit* room_search_ = nullptr;
    QLineEdit* room_name_ = nullptr;
    QTextEdit* room_desc_ = nullptr;
    QSpinBox* room_sector_ = nullptr;
    QSpinBox* room_flags_ = nullptr;
    QListWidget* exit_list_ = nullptr;
    QComboBox* exit_direction_ = nullptr;
    QSpinBox* exit_to_room_ = nullptr;
    QLineEdit* exit_description_ = nullptr;
    QLineEdit* exit_keyword_ = nullptr;
    QSpinBox* exit_info_ = nullptr;
    QSpinBox* exit_key_ = nullptr;

    QListWidget* mob_list_ = nullptr;
    QLineEdit* mob_search_ = nullptr;
    QLineEdit* mob_short_ = nullptr;
    QTextEdit* mob_long_ = nullptr;
    QSpinBox* mob_level_ = nullptr;
    QSpinBox* mob_alignment_ = nullptr;

    QListWidget* obj_list_ = nullptr;
    QLineEdit* obj_search_ = nullptr;
    QLineEdit* obj_short_ = nullptr;
    QTextEdit* obj_desc_ = nullptr;
    QSpinBox* obj_cost_ = nullptr;
    QSpinBox* obj_weight_ = nullptr;

    QPlainTextEdit* validation_log_ = nullptr;
    QWidget* validation_tab_ = nullptr;
};

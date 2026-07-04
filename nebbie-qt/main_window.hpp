#pragma once

#include "nebbie/lib_context.hpp"
#include "nebbie/session.hpp"
#include "nebbie/validate.hpp"
#include "nebbie/world.hpp"

#include <QMainWindow>
#include <QTimer>

#include <chrono>
#include <filesystem>
#include <vector>

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
class QListWidgetItem;
class ZoneMapWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    void openLibPath(const QString& path);
    void openStartupLib();
    bool promptForLibPath(const QString& reason = {});

public slots:
    void openLib();
    void saveLib();
    void saveLibForce();
    void validateLib();
    void onAutosaveTick();
    void restoreFromWorkspace();
    void restoreVersion();
    void onValidationIssueActivated(QListWidgetItem* item);

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
    void onZoneSelected();
    void onResetSelected();
    void onResetCommandChanged();
    void addZoneReset();
    void applyResetChanges();
    void removeSelectedReset();
    void moveResetUp();
    void moveResetDown();
    void goToResetRoom();
    void goToResetEntity();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUi();
    void setupMenus();
    void loadLib(const std::filesystem::path& path);
    void rememberLibPath(const std::filesystem::path& path);
    void refreshRoomList();
    void refreshMobList();
    void refreshObjectList();
    void refreshExitList(long room_vnum);
    void refreshZoneList();
    void refreshZoneMap();
    void updateMapStats();
    void refreshResetList(int zone_num);
    void updateResetFieldHints();
    void loadResetForm(const nebbie::ResetCommand& cmd);
    nebbie::ResetCommand readResetForm() const;
    int currentZoneNum() const;
    int currentResetIndex() const;
    void selectZoneByNum(int zone_num);
    void selectRoomByVnum(long vnum);
    void selectMobByVnum(long vnum);
    void selectObjectByVnum(long vnum);
    long currentRoomVnum() const;
    void setStatus(const QString& message);
    void showValidation(const nebbie::ValidationReport& report);
    void navigateToIssue(const nebbie::ValidationIssue& issue);
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

    QListWidget* zone_list_ = nullptr;
    QLabel* zone_info_ = nullptr;
    QListWidget* reset_list_ = nullptr;
    QComboBox* reset_command_ = nullptr;
    QLabel* reset_hint_ = nullptr;
    QSpinBox* reset_if_flag_ = nullptr;
    QSpinBox* reset_arg1_ = nullptr;
    QSpinBox* reset_arg2_ = nullptr;
    QSpinBox* reset_arg3_ = nullptr;
    QSpinBox* reset_arg4_ = nullptr;
    QPushButton* reset_apply_ = nullptr;
    QPushButton* reset_remove_ = nullptr;

    nebbie::SessionConfig session_config_;
    std::chrono::system_clock::time_point last_version_time_{};
    QTimer* autosave_timer_ = nullptr;

    QListWidget* validation_list_ = nullptr;
    std::vector<nebbie::ValidationIssue> validation_issues_;
    QWidget* validation_tab_ = nullptr;
    QWidget* zone_tab_ = nullptr;
    QWidget* map_tab_ = nullptr;
    QComboBox* map_zone_ = nullptr;
    QComboBox* map_floor_ = nullptr;
    ZoneMapWidget* map_view_ = nullptr;
    QLabel* map_stats_ = nullptr;
};

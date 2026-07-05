#pragma once

#include "app_config.hpp"
#include "nebbie/lib_context.hpp"
#include "nebbie/session.hpp"
#include "nebbie/validate.hpp"
#include "nebbie/world.hpp"
#include "nebbie/world_index.hpp"

#include <QMainWindow>
#include <QTimer>

#include <chrono>
#include <filesystem>
#include <optional>
#include <vector>

class QNetworkAccessManager;

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
class WorldZoneMapWidget;
class QCheckBox;
class MobEditorWidget;
class ObjEditorWidget;
class RoomEditorWidget;
class ZoneEditorWidget;

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
    void applyZoneChanges();
    void createRoom();
    void createMob();
    void createObject();
    void onRoomSearchChanged(const QString& text);
    void onMobSearchChanged(const QString& text);
    void onObjSearchChanged(const QString& text);
    void goToExitTarget();
    void onZoneSelected();
    void configureCoordinator();
    void refreshWorldIndex();
    void loadWorldIndexFromFile();
    void exportLocalWorldIndex();
    void exportOverlays();
    void reserveVnums();

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
    void refreshZoneList();
    void refreshZoneMap();
    void refreshWorldZoneMap();
    void updateMapStats();
    void openZoneRoomMap(int zone_num);
    void exportMapPng(ZoneMapWidget* view, const QString& suggested_name);
    void exportMapPng(WorldZoneMapWidget* view, const QString& suggested_name);
    void updateWorldZoneDetails(int zone_num);
    int currentZoneNum() const;
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
    int preferredZoneNumForNewRoom() const;
    long suggestRoomVnum() const;
    long suggestMobVnum() const;
    long suggestObjectVnum() const;
    bool warnIfRemoteVnumConflict(const QString& kind, long vnum) const;
    nebbie::WorldIndex worldIndexForValidation() const;

    nebbie::World world_;
    nebbie::LibContext context_;
    std::filesystem::path lib_path_;
    bool dirty_ = false;
    nebbie::qt::AppConfig app_config_;
    std::optional<nebbie::WorldIndex> world_index_;
    QNetworkAccessManager* network_ = nullptr;

    QString room_filter_;
    QString mob_filter_;
    QString object_filter_;

    QLabel* lib_label_ = nullptr;
    QTabWidget* tabs_ = nullptr;

    QListWidget* room_list_ = nullptr;
    QLineEdit* room_search_ = nullptr;
    RoomEditorWidget* room_editor_ = nullptr;

    QListWidget* mob_list_ = nullptr;
    QLineEdit* mob_search_ = nullptr;
    MobEditorWidget* mob_editor_ = nullptr;

    QListWidget* obj_list_ = nullptr;
    QLineEdit* obj_search_ = nullptr;
    ObjEditorWidget* obj_editor_ = nullptr;

    QListWidget* zone_list_ = nullptr;
    ZoneEditorWidget* zone_editor_ = nullptr;

    nebbie::SessionConfig session_config_;
    std::chrono::system_clock::time_point last_version_time_{};
    QTimer* autosave_timer_ = nullptr;

    QListWidget* validation_list_ = nullptr;
    std::vector<nebbie::ValidationIssue> validation_issues_;
    QWidget* validation_tab_ = nullptr;
    QWidget* zone_tab_ = nullptr;
    QWidget* map_tab_ = nullptr;
    QTabWidget* map_tabs_ = nullptr;
    int selected_world_zone_ = -1;
    QComboBox* map_zone_ = nullptr;
    QComboBox* map_floor_ = nullptr;
    QCheckBox* map_broken_only_ = nullptr;
    ZoneMapWidget* map_view_ = nullptr;
    QLabel* map_stats_ = nullptr;
    WorldZoneMapWidget* world_map_view_ = nullptr;
    QPlainTextEdit* world_map_details_ = nullptr;
    QCheckBox* world_map_broken_only_ = nullptr;
    QLabel* world_map_stats_ = nullptr;
};

#pragma once

#include "nebbie/zone_graph.hpp"

#include <QGraphicsView>

class QGraphicsScene;
class QMouseEvent;
class QWheelEvent;

class ZoneMapWidget : public QGraphicsView {
    Q_OBJECT

public:
    explicit ZoneMapWidget(QWidget* parent = nullptr);

    void setGraph(const nebbie::ZoneGraph& graph);
    void clearGraph();
    void setActiveZLevel(int z);
    int activeZLevel() const;
    std::vector<int> availableZLevels() const;
    void setShowBrokenOnly(bool enabled);
    void setHighlightedVnum(long vnum);

signals:
    void roomActivated(long vnum);
    void floorLinkActivated(long target_vnum, int target_z);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    struct NodeItem;
    struct VerticalBadge;

    void rebuildScene();
    long vnumAt(const QPointF& scene_pos) const;
    bool verticalBadgeAt(const QPointF& scene_pos, long& target_vnum, int& target_z) const;

    QGraphicsScene* scene_ = nullptr;
    nebbie::ZoneGraph graph_;
    nebbie::ZoneZLayout z_layout_;
    int active_z_ = 0;
    bool show_broken_only_ = false;
    long highlighted_vnum_ = -1;
    QHash<long, NodeItem*> nodes_by_vnum_;
    QList<VerticalBadge*> vertical_badges_;
};

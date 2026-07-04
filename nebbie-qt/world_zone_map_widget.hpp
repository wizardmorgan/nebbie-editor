#pragma once

#include "nebbie/zone_graph.hpp"

#include <QGraphicsView>

class QGraphicsScene;
class QMouseEvent;
class QWheelEvent;

class WorldZoneMapWidget : public QGraphicsView {
    Q_OBJECT

public:
    explicit WorldZoneMapWidget(QWidget* parent = nullptr);

    void setGraph(const nebbie::WorldZoneGraph& graph);
    void clearGraph();
    void setShowBrokenOnly(bool enabled);
    void setHighlightedZone(int zone_num);
    bool exportSceneToPng(const QString& path) const;

signals:
    void zoneActivated(int zone_num);
    void zoneSelected(int zone_num);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    struct ZoneNodeItem;

    void rebuildScene();
    int zoneAt(const QPointF& scene_pos) const;

    QGraphicsScene* scene_ = nullptr;
    nebbie::WorldZoneGraph graph_;
    bool show_broken_only_ = false;
    int highlighted_zone_ = -1;
    QHash<int, ZoneNodeItem*> nodes_by_zone_;
};

#pragma once

#include "nebbie/zone_graph.hpp"

#include <QGraphicsView>

class QGraphicsScene;
class QWheelEvent;

class ZoneMapWidget : public QGraphicsView {
    Q_OBJECT

public:
    explicit ZoneMapWidget(QWidget* parent = nullptr);

    void setGraph(const nebbie::ZoneGraph& graph);
    void clearGraph();

signals:
    void roomActivated(long vnum);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    struct NodeItem;

    void rebuildScene(const nebbie::ZoneGraph& graph);
    long vnumAt(const QPointF& scene_pos) const;

    QGraphicsScene* scene_ = nullptr;
    nebbie::ZoneGraph graph_;
    QHash<long, NodeItem*> nodes_by_vnum_;
};

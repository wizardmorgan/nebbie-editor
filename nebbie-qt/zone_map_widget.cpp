#include "zone_map_widget.hpp"

#include "nebbie/edit.hpp"

#include <QBrush>
#include <QFont>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QMouseEvent>
#include <QPen>
#include <QWheelEvent>

#include <map>
#include <queue>
#include <unordered_map>

namespace {

constexpr qreal kNodeWidth = 96.0;
constexpr qreal kNodeHeight = 44.0;
constexpr qreal kGridStep = 130.0;

constexpr int kDataVnum = 0;
constexpr int kDataVerticalLink = 1;
constexpr int kDataTargetVnum = 2;
constexpr int kDataTargetZ = 3;

struct GridPos {
    int x = 0;
    int y = 0;

    bool operator<(const GridPos& other) const {
        if (x != other.x) {
            return x < other.x;
        }
        return y < other.y;
    }
};

GridPos direction_delta(int direction) {
    switch (direction) {
    case 0:
        return {0, -1};
    case 1:
        return {1, 0};
    case 2:
        return {0, 1};
    case 3:
        return {-1, 0};
    default:
        return {0, 0};
    }
}

bool is_horizontal_direction(int direction) {
    return direction >= 0 && direction <= 3;
}

int level_of(const nebbie::ZoneZLayout& layout, long vnum, int fallback = 0) {
    const auto it = layout.levels.find(vnum);
    return it == layout.levels.end() ? fallback : it->second;
}

std::map<long, GridPos> layout_horizontal_nodes(const nebbie::ZoneGraph& graph,
                                                const nebbie::ZoneZLayout& z_layout,
                                                int active_z) {
    std::map<long, GridPos> positions;
    if (graph.nodes.empty()) {
        return positions;
    }

    long seed = -1;
    for (const auto& node : graph.nodes) {
        if (level_of(z_layout, node.vnum) != active_z) {
            continue;
        }
        if (seed < 0 || node.vnum < seed) {
            seed = node.vnum;
        }
    }
    if (seed < 0) {
        return positions;
    }

    std::map<GridPos, long> occupied;
    std::queue<long> pending;
    pending.push(seed);
    positions[seed] = {0, 0};
    occupied[{0, 0}] = seed;

    std::unordered_map<long, bool> seen;
    seen[seed] = true;

    auto find_free_cell = [&](GridPos desired) {
        if (!occupied.count(desired)) {
            return desired;
        }
        for (int radius = 1; radius < 32; ++radius) {
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dy = -radius; dy <= radius; ++dy) {
                    const GridPos candidate{desired.x + dx, desired.y + dy};
                    if (!occupied.count(candidate)) {
                        return candidate;
                    }
                }
            }
        }
        return desired;
    };

    while (!pending.empty()) {
        const long from_vnum = pending.front();
        pending.pop();
        const GridPos from_pos = positions[from_vnum];

        for (const auto& edge : graph.edges) {
            if (edge.from_vnum != from_vnum || edge.to_vnum <= 0 || !is_horizontal_direction(edge.direction)) {
                continue;
            }
            if (level_of(z_layout, edge.from_vnum) != active_z
                || level_of(z_layout, edge.to_vnum) != active_z) {
                continue;
            }
            if (seen[edge.to_vnum]) {
                continue;
            }

            const GridPos delta = direction_delta(edge.direction);
            GridPos target{from_pos.x + delta.x, from_pos.y + delta.y};
            target = find_free_cell(target);

            seen[edge.to_vnum] = true;
            positions[edge.to_vnum] = target;
            occupied[target] = edge.to_vnum;
            pending.push(edge.to_vnum);
        }
    }

    int orphan = 0;
    for (const auto& node : graph.nodes) {
        if (level_of(z_layout, node.vnum) != active_z || positions.count(node.vnum)) {
            continue;
        }
        const GridPos isolated = find_free_cell({orphan++, 0});
        positions[node.vnum] = isolated;
        occupied[isolated] = node.vnum;
    }

    return positions;
}

QColor direction_color(int direction) {
    switch (direction) {
    case 0:
        return QColor(46, 125, 50);
    case 1:
        return QColor(25, 118, 210);
    case 2:
        return QColor(198, 40, 40);
    case 3:
        return QColor(245, 124, 0);
    case 4:
    case 5:
        return QColor(106, 27, 154);
    default:
        return QColor(96, 96, 96);
    }
}

} // namespace

struct ZoneMapWidget::NodeItem {
    QGraphicsRectItem* rect = nullptr;
    QGraphicsSimpleTextItem* label = nullptr;
    long vnum = 0;
};

struct ZoneMapWidget::VerticalBadge {
    QGraphicsSimpleTextItem* item = nullptr;
    long target_vnum = 0;
    int target_z = 0;
};

ZoneMapWidget::ZoneMapWidget(QWidget* parent) : QGraphicsView(parent) {
    scene_ = new QGraphicsScene(this);
    setScene(scene_);
    setRenderHint(QPainter::Antialiasing, true);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
}

void ZoneMapWidget::clearGraph() {
    graph_ = {};
    z_layout_ = {};
    active_z_ = 0;
    nodes_by_vnum_.clear();
    vertical_badges_.clear();
    scene_->clear();
}

void ZoneMapWidget::setGraph(const nebbie::ZoneGraph& graph) {
    graph_ = graph;
    z_layout_ = nebbie::compute_zone_z_levels(graph_);
    active_z_ = 0;
    rebuildScene();
}

void ZoneMapWidget::setActiveZLevel(int z) {
    if (active_z_ == z) {
        return;
    }
    active_z_ = z;
    rebuildScene();
}

int ZoneMapWidget::activeZLevel() const {
    return active_z_;
}

std::vector<int> ZoneMapWidget::availableZLevels() const {
    return nebbie::sorted_z_levels(z_layout_);
}

void ZoneMapWidget::rebuildScene() {
    nodes_by_vnum_.clear();
    vertical_badges_.clear();
    scene_->clear();

    if (graph_.nodes.empty()) {
        return;
    }

    const auto positions = layout_horizontal_nodes(graph_, z_layout_, active_z_);
    std::unordered_map<long, QPointF> centers;

    for (const auto& node : graph_.nodes) {
        if (level_of(z_layout_, node.vnum) != active_z_) {
            continue;
        }

        const auto it = positions.find(node.vnum);
        if (it == positions.end()) {
            continue;
        }

        const qreal px = it->second.x * kGridStep;
        const qreal py = it->second.y * kGridStep;
        const QRectF rect(px - kNodeWidth / 2.0, py - kNodeHeight / 2.0, kNodeWidth, kNodeHeight);

        auto* node_item = new NodeItem;
        node_item->vnum = node.vnum;
        node_item->rect = scene_->addRect(rect, QPen(QColor(55, 55, 55)), QBrush(QColor(245, 248, 252)));
        node_item->rect->setZValue(2);
        node_item->rect->setData(kDataVnum, static_cast<qlonglong>(node.vnum));

        const QString title = QString("#%1").arg(node.vnum);
        const QString subtitle = QString::fromStdString(node.name);
        node_item->label = scene_->addSimpleText(title + "\n" + subtitle.left(16));
        node_item->label->setPos(rect.x() + 6, rect.y() + 4);
        node_item->label->setZValue(3);
        node_item->label->setData(kDataVnum, static_cast<qlonglong>(node.vnum));

        QString tip = QString("#%1 %2\nSettore %3\nPiano Z=%4")
                          .arg(node.vnum)
                          .arg(QString::fromStdString(node.name))
                          .arg(node.sector_type)
                          .arg(active_z_);

        QStringList vertical_links;
        for (const auto& edge : graph_.edges) {
            if (edge.from_vnum != node.vnum || edge.to_vnum <= 0 || edge.direction < 4) {
                continue;
            }
            const int target_z = level_of(z_layout_, edge.to_vnum, active_z_);
            const char* dir = nebbie::exit_direction_name(edge.direction);
            vertical_links << QString("%1 → #%2 (Z=%3)").arg(QString::fromUtf8(dir)).arg(edge.to_vnum).arg(target_z);

            auto* badge = new VerticalBadge;
            badge->target_vnum = edge.to_vnum;
            badge->target_z = target_z;
            const QString badge_text = QString::fromUtf8(dir) + QString(" #%1").arg(edge.to_vnum);
            badge->item = scene_->addSimpleText(badge_text);
            QFont font = badge->item->font();
            font.setPointSizeF(font.pointSizeF() * 0.85);
            font.setUnderline(true);
            badge->item->setFont(font);
            badge->item->setBrush(direction_color(edge.direction));
            badge->item->setZValue(5);
            badge->item->setData(kDataVerticalLink, 1);
            badge->item->setData(kDataTargetVnum, static_cast<qlonglong>(edge.to_vnum));
            badge->item->setData(kDataTargetZ, target_z);
            badge->item->setToolTip(QString("Vai al piano Z=%1, stanza #%2").arg(target_z).arg(edge.to_vnum));

            const qreal badge_y = edge.direction == 4 ? rect.top() - 18 : rect.bottom() + 2;
            badge->item->setPos(rect.center().x() - 28, badge_y);
            vertical_badges_.push_back(badge);
        }

        if (!vertical_links.isEmpty()) {
            tip += "\n\nCollegamenti verticali:\n" + vertical_links.join("\n");
        }
        node_item->rect->setToolTip(tip);

        nodes_by_vnum_.insert(node.vnum, node_item);
        centers[node.vnum] = rect.center();
    }

    for (const auto& edge : graph_.edges) {
        if (edge.to_vnum <= 0 || !is_horizontal_direction(edge.direction)) {
            continue;
        }
        if (level_of(z_layout_, edge.from_vnum) != active_z_ || level_of(z_layout_, edge.to_vnum) != active_z_) {
            continue;
        }

        const auto from_it = centers.find(edge.from_vnum);
        const auto to_it = centers.find(edge.to_vnum);
        if (from_it == centers.end() || to_it == centers.end()) {
            continue;
        }

        QPen pen(direction_color(edge.direction), edge.broken ? 2.0 : 1.5);
        if (edge.broken) {
            pen.setStyle(Qt::DashLine);
            pen.setColor(Qt::red);
        }

        auto* line = scene_->addLine(QLineF(from_it->second, to_it->second), pen);
        line->setZValue(1);

        const QPointF mid = (from_it->second + to_it->second) / 2.0;
        auto* dir_label = scene_->addSimpleText(QString::fromUtf8(nebbie::exit_direction_name(edge.direction)));
        dir_label->setBrush(direction_color(edge.direction));
        dir_label->setPos(mid);
        dir_label->setZValue(4);
    }

    auto* floor_label = scene_->addSimpleText(QString("Piano Z = %1").arg(active_z_));
    QFont floor_font = floor_label->font();
    floor_font.setBold(true);
    floor_label->setFont(floor_font);
    floor_label->setZValue(10);
    floor_label->setPos(scene_->itemsBoundingRect().topLeft() + QPointF(0, -30));

    scene_->setSceneRect(scene_->itemsBoundingRect().adjusted(-80, -80, 80, 80));
}

long ZoneMapWidget::vnumAt(const QPointF& scene_pos) const {
    const auto items = scene_->items(scene_pos);
    for (QGraphicsItem* item : items) {
        const QVariant data = item->data(kDataVnum);
        if (data.isValid()) {
            return data.toLongLong();
        }
    }
    return -1;
}

bool ZoneMapWidget::verticalBadgeAt(const QPointF& scene_pos, long& target_vnum, int& target_z) const {
    const auto items = scene_->items(scene_pos);
    for (QGraphicsItem* item : items) {
        if (!item->data(kDataVerticalLink).toInt()) {
            continue;
        }
        target_vnum = item->data(kDataTargetVnum).toLongLong();
        target_z = item->data(kDataTargetZ).toInt();
        return target_vnum > 0;
    }
    return false;
}

void ZoneMapWidget::mousePressEvent(QMouseEvent* event) {
    long target_vnum = 0;
    int target_z = 0;
    if (verticalBadgeAt(mapToScene(event->pos()), target_vnum, target_z)) {
        emit floorLinkActivated(target_vnum, target_z);
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void ZoneMapWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    const long vnum = vnumAt(mapToScene(event->pos()));
    if (vnum > 0) {
        emit roomActivated(vnum);
        event->accept();
        return;
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void ZoneMapWidget::wheelEvent(QWheelEvent* event) {
    const qreal factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    scale(factor, factor);
}

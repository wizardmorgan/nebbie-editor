#include "world_zone_map_widget.hpp"

#include <QBrush>
#include <QFont>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QTextDocument>
#include <QWheelEvent>

#include <map>
#include <queue>
#include <unordered_map>

namespace {

constexpr qreal kMinZoneWidth = 152.0;
constexpr qreal kMinZoneHeight = 86.0;
constexpr qreal kGridStepX = 220.0;
constexpr qreal kGridStepY = 140.0;
constexpr qreal kNodePadding = 10.0;
constexpr int kDataZone = 0;

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

QString truncate_zone_name(const std::string& name, int max_chars = 24) {
    QString qname = QString::fromStdString(name).trimmed();
    if (qname.size() <= max_chars) {
        return qname;
    }
    return qname.left(max_chars - 1) + QChar(0x2026);
}

QString format_zone_label(const nebbie::WorldZoneNode& zone) {
    return QString("#%1  %2\n[%3\u2013%4]\nUsati: %5   Liberi: %6")
        .arg(zone.zone_num)
        .arg(truncate_zone_name(zone.name))
        .arg(zone.bottom)
        .arg(zone.top)
        .arg(zone.used_count)
        .arg(zone.free_count);
}

QSizeF measure_zone_label(const QString& text, qreal text_width) {
    QTextDocument doc;
    QFont font;
    font.setPointSize(9);
    doc.setDefaultFont(font);
    doc.setTextWidth(text_width);
    doc.setPlainText(text);
    return QSizeF(doc.idealWidth(), doc.size().height());
}

std::map<int, GridPos> layout_zone_nodes(const nebbie::WorldZoneGraph& graph) {
    std::map<int, GridPos> positions;
    if (graph.zones.empty()) {
        return positions;
    }

    int seed = graph.zones.front().zone_num;
    for (const auto& zone : graph.zones) {
        seed = std::min(seed, zone.zone_num);
    }

    std::map<GridPos, int> occupied;
    std::queue<int> pending;
    pending.push(seed);
    positions[seed] = {0, 0};
    occupied[{0, 0}] = seed;

    std::unordered_map<int, bool> seen;
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
        const int from_zone = pending.front();
        pending.pop();
        const GridPos from_pos = positions[from_zone];

        for (const auto& edge : graph.edges) {
            if (edge.from_zone != from_zone) {
                continue;
            }
            if (seen[edge.to_zone]) {
                continue;
            }

            GridPos target{from_pos.x + 1, from_pos.y};
            target = find_free_cell(target);
            seen[edge.to_zone] = true;
            positions[edge.to_zone] = target;
            occupied[target] = edge.to_zone;
            pending.push(edge.to_zone);
        }
    }

    int orphan = 0;
    for (const auto& zone : graph.zones) {
        if (positions.count(zone.zone_num)) {
            continue;
        }
        const GridPos isolated = find_free_cell({orphan++, 0});
        positions[zone.zone_num] = isolated;
        occupied[isolated] = zone.zone_num;
    }

    return positions;
}

} // namespace

struct WorldZoneMapWidget::ZoneNodeItem {
    QGraphicsRectItem* rect = nullptr;
    QGraphicsTextItem* label = nullptr;
    int zone_num = 0;
};

WorldZoneMapWidget::WorldZoneMapWidget(QWidget* parent) : QGraphicsView(parent) {
    scene_ = new QGraphicsScene(this);
    setScene(scene_);
    setRenderHint(QPainter::Antialiasing, true);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
}

void WorldZoneMapWidget::clearGraph() {
    graph_ = {};
    nodes_by_zone_.clear();
    scene_->clear();
}

void WorldZoneMapWidget::setGraph(const nebbie::WorldZoneGraph& graph) {
    graph_ = graph;
    rebuildScene();
}

void WorldZoneMapWidget::setShowBrokenOnly(bool enabled) {
    if (show_broken_only_ == enabled) {
        return;
    }
    show_broken_only_ = enabled;
    rebuildScene();
}

void WorldZoneMapWidget::setHighlightedZone(int zone_num) {
    if (highlighted_zone_ == zone_num) {
        return;
    }
    highlighted_zone_ = zone_num;
    rebuildScene();
}

void WorldZoneMapWidget::rebuildScene() {
    nodes_by_zone_.clear();
    scene_->clear();

    if (graph_.zones.empty()) {
        return;
    }

    const auto positions = layout_zone_nodes(graph_);
    std::unordered_map<int, QPointF> centers;

    for (const auto& zone : graph_.zones) {
        const auto it = positions.find(zone.zone_num);
        if (it == positions.end()) {
            continue;
        }

        const QString label_text = format_zone_label(zone);
        const qreal inner_width = kMinZoneWidth - (2.0 * kNodePadding);
        const QSizeF text_size = measure_zone_label(label_text, inner_width);
        const qreal box_w = qMax(kMinZoneWidth, text_size.width() + (2.0 * kNodePadding));
        const qreal box_h = qMax(kMinZoneHeight, text_size.height() + (2.0 * kNodePadding));

        const qreal px = it->second.x * kGridStepX;
        const qreal py = it->second.y * kGridStepY;
        const QRectF rect(px - box_w / 2.0, py - box_h / 2.0, box_w, box_h);

        auto* node_item = new ZoneNodeItem;
        node_item->zone_num = zone.zone_num;
        const bool highlighted = highlighted_zone_ == zone.zone_num;
        const QPen pen(highlighted ? QColor(220, 120, 0) : QColor(55, 55, 55), highlighted ? 3.0 : 1.0);
        const QBrush brush(highlighted ? QColor(255, 248, 220) : QColor(248, 252, 248));
        node_item->rect = scene_->addRect(rect, pen, brush);
        node_item->rect->setZValue(2);
        node_item->rect->setData(kDataZone, zone.zone_num);

        node_item->label = scene_->addText(label_text);
        QFont label_font = node_item->label->font();
        label_font.setPointSize(9);
        node_item->label->setFont(label_font);
        node_item->label->setDefaultTextColor(QColor(25, 25, 25));
        node_item->label->setTextWidth(inner_width);
        const QRectF text_rect = node_item->label->boundingRect();
        const qreal text_x = rect.x() + (rect.width() - text_rect.width()) / 2.0;
        const qreal text_y = rect.y() + (rect.height() - text_rect.height()) / 2.0;
        node_item->label->setPos(text_x, text_y);
        node_item->label->setZValue(3);
        node_item->label->setData(kDataZone, zone.zone_num);

        const QString used_list = zone.used_vnums.empty()
            ? QStringLiteral("(nessuna stanza)")
            : QString::fromStdString(
                  std::to_string(zone.used_vnums.front())
                  + (zone.used_vnums.size() > 1
                         ? " … " + std::to_string(zone.used_vnums.back())
                         : ""));

        node_item->rect->setToolTip(
            QString("#%1 %2\nRange: %3-%4\nStanze usate: %5\nVnum liberi: %6\nLiberi (range): %7\nUsati (campione): %8")
                .arg(zone.zone_num)
                .arg(QString::fromStdString(zone.name))
                .arg(zone.bottom)
                .arg(zone.top)
                .arg(zone.used_count)
                .arg(zone.free_count)
                .arg(QString::fromStdString(nebbie::format_vnum_ranges(zone.free_ranges, 12)))
                .arg(used_list));

        nodes_by_zone_.insert(zone.zone_num, node_item);
        centers[zone.zone_num] = rect.center();
    }

    for (const auto& edge : graph_.edges) {
        if (show_broken_only_ && edge.broken_count == 0) {
            continue;
        }

        const auto from_it = centers.find(edge.from_zone);
        const auto to_it = centers.find(edge.to_zone);
        if (from_it == centers.end() || to_it == centers.end()) {
            continue;
        }

        QPen pen(edge.broken_count > 0 ? QColor(200, 40, 40) : QColor(60, 90, 160), 2.0);
        if (edge.broken_count > 0) {
            pen.setStyle(Qt::DashLine);
        }

        auto* line = scene_->addLine(QLineF(from_it->second, to_it->second), pen);
        line->setZValue(1);

        const QPointF mid = (from_it->second + to_it->second) / 2.0;
        const QPointF delta = to_it->second - from_it->second;
        QPointF offset(0, 0);
        if (std::abs(delta.x()) >= std::abs(delta.y())) {
            offset = QPointF(0, -14);
        } else {
            offset = QPointF(14, 0);
        }

        QString label = QString("%1 link").arg(edge.link_count);
        if (edge.broken_count > 0) {
            label += QString(", %1 rotti").arg(edge.broken_count);
        }

        auto* edge_label = scene_->addText(label);
        QFont edge_font = edge_label->font();
        edge_font.setPointSize(8);
        edge_label->setFont(edge_font);
        edge_label->setDefaultTextColor(edge.broken_count > 0 ? QColor(170, 30, 30) : QColor(50, 70, 120));
        const QRectF edge_bounds = edge_label->boundingRect();
        const QPointF label_pos = mid + offset - QPointF(edge_bounds.width() / 2.0, edge_bounds.height() / 2.0);

        auto* edge_bg = scene_->addRect(edge_bounds.translated(label_pos), QPen(QColor(220, 225, 235)), QBrush(QColor(255, 255, 255, 230)));
        edge_bg->setZValue(3);
        edge_label->setPos(label_pos);
        edge_label->setZValue(4);
    }

    scene_->setSceneRect(scene_->itemsBoundingRect().adjusted(-100, -100, 100, 100));
}

int WorldZoneMapWidget::zoneAt(const QPointF& scene_pos) const {
    const auto items = scene_->items(scene_pos);
    for (QGraphicsItem* item : items) {
        const QVariant data = item->data(kDataZone);
        if (data.isValid()) {
            return data.toInt();
        }
    }
    return -1;
}

void WorldZoneMapWidget::mousePressEvent(QMouseEvent* event) {
    const int zone_num = zoneAt(mapToScene(event->pos()));
    if (zone_num >= 0) {
        setHighlightedZone(zone_num);
        emit zoneSelected(zone_num);
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void WorldZoneMapWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    const int zone_num = zoneAt(mapToScene(event->pos()));
    if (zone_num >= 0) {
        emit zoneActivated(zone_num);
        event->accept();
        return;
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void WorldZoneMapWidget::wheelEvent(QWheelEvent* event) {
    const qreal factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    scale(factor, factor);
}

bool WorldZoneMapWidget::exportSceneToPng(const QString& path) const {
    if (!scene_ || scene_->items().isEmpty()) {
        return false;
    }

    const QRectF bounds = scene_->itemsBoundingRect().adjusted(-48, -48, 48, 48);
    const QSize size = bounds.size().toSize().expandedTo(QSize(800, 600));
    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    scene_->render(&painter, QRectF(QPointF(0, 0), size), bounds);
    painter.end();

    return image.save(path);
}

#include "world_zone_map_widget.hpp"

#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QWheelEvent>

#include <algorithm>
#include <map>
#include <queue>
#include <unordered_map>
#include <vector>

namespace {

constexpr qreal kZonePadding = 10.0;
constexpr qreal kNodeMargin = 48.0;
constexpr qreal kMinZoneBoxWidth = 168.0;
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

struct ZoneLayoutInfo {
    int zone_num = 0;
    QStringList lines;
    QSizeF box_size;
};

QFont zone_label_font() {
    QFont font;
    font.setPointSize(9);
    return font;
}

QString truncate_zone_name(const std::string& name, int max_chars = 16) {
    QString qname = QString::fromStdString(name).trimmed();
    if (qname.size() <= max_chars) {
        return qname;
    }
    return qname.left(max_chars - 1) + QChar(0x2026);
}

QStringList format_zone_lines(const nebbie::WorldZoneNode& zone) {
    return {
        QString("#%1  %2").arg(zone.zone_num).arg(truncate_zone_name(zone.name)),
        QString("[%1\u2013%2]").arg(zone.bottom).arg(zone.top),
        QString("Usati: %1").arg(zone.used_count),
        QString("Liberi: %1").arg(zone.free_count),
    };
}

QString elide_line(const QFontMetrics& fm, const QString& line, qreal max_width) {
    return fm.elidedText(line, Qt::ElideRight, static_cast<int>(max_width));
}

ZoneLayoutInfo measure_zone_layout(const nebbie::WorldZoneNode& zone, qreal uniform_inner_width) {
    ZoneLayoutInfo info;
    info.zone_num = zone.zone_num;
    info.lines = format_zone_lines(zone);

    const QFont font = zone_label_font();
    const QFontMetrics fm(font);
    const qreal line_step = fm.lineSpacing() + 2.0;

    for (QString& line : info.lines) {
        line = elide_line(fm, line, uniform_inner_width);
    }

    const qreal box_w = uniform_inner_width + (2.0 * kZonePadding);
    const qreal box_h = (2.0 * kZonePadding) + (info.lines.size() * line_step);
    info.box_size = QSizeF(box_w, box_h);
    return info;
}

QRectF rect_centered_at(const QPointF& center, const QSizeF& size) {
    return QRectF(center.x() - size.width() / 2.0,
                  center.y() - size.height() / 2.0,
                  size.width(),
                  size.height());
}

bool separate_rect_pair(QRectF& a, QRectF& b, qreal margin) {
    const QRectF expanded_a = a.adjusted(-margin, -margin, margin, margin);
    const QRectF expanded_b = b.adjusted(-margin, -margin, margin, margin);
    if (!expanded_a.intersects(expanded_b)) {
        return false;
    }

    const qreal overlap_left = expanded_b.right() - expanded_a.left();
    const qreal overlap_right = expanded_a.right() - expanded_b.left();
    const qreal overlap_top = expanded_b.bottom() - expanded_a.top();
    const qreal overlap_bottom = expanded_a.bottom() - expanded_b.top();

    const qreal overlap_x = qMin(overlap_left, overlap_right);
    const qreal overlap_y = qMin(overlap_top, overlap_bottom);

    if (overlap_x <= 0.0 || overlap_y <= 0.0) {
        return false;
    }

    if (overlap_x <= overlap_y) {
        const qreal shift = overlap_x + 2.0;
        if (a.center().x() <= b.center().x()) {
            a.translate(-shift / 2.0, 0.0);
            b.translate(shift / 2.0, 0.0);
        } else {
            a.translate(shift / 2.0, 0.0);
            b.translate(-shift / 2.0, 0.0);
        }
    } else {
        const qreal shift = overlap_y + 2.0;
        if (a.center().y() <= b.center().y()) {
            a.translate(0.0, -shift / 2.0);
            b.translate(0.0, shift / 2.0);
        } else {
            a.translate(0.0, shift / 2.0);
            b.translate(0.0, -shift / 2.0);
        }
    }
    return true;
}

void resolve_rect_overlaps(std::vector<QRectF>& rects, qreal margin) {
    if (rects.size() < 2) {
        return;
    }

    for (int pass = 0; pass < 400; ++pass) {
        bool moved = false;
        for (std::size_t i = 0; i < rects.size(); ++i) {
            if (rects[i].isNull()) {
                continue;
            }
            for (std::size_t j = i + 1; j < rects.size(); ++j) {
                if (rects[j].isNull()) {
                    continue;
                }
                if (separate_rect_pair(rects[i], rects[j], margin)) {
                    moved = true;
                }
            }
        }
        if (!moved) {
            break;
        }
    }
}

void add_zone_line_labels(QGraphicsScene* scene,
                          const QRectF& rect,
                          int zone_num,
                          const QStringList& lines) {
    const QFont font = zone_label_font();
    const QFontMetrics fm(font);
    const qreal line_step = fm.lineSpacing() + 2.0;
    qreal y = rect.y() + kZonePadding;

    for (const QString& line : lines) {
        auto* item = scene->addSimpleText(line);
        item->setFont(font);
        item->setBrush(QColor(25, 25, 25));
        item->setPos(rect.x() + kZonePadding, y);
        item->setZValue(3);
        item->setData(kDataZone, zone_num);
        y += line_step;
    }
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
        for (int radius = 1; radius < 48; ++radius) {
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

            GridPos target{from_pos.x + 2, from_pos.y};
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
        const GridPos isolated = find_free_cell({0, orphan++});
        positions[zone.zone_num] = isolated;
        occupied[isolated] = zone.zone_num;
    }

    return positions;
}

} // namespace

struct WorldZoneMapWidget::ZoneNodeItem {
    QGraphicsRectItem* rect = nullptr;
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

    std::vector<ZoneLayoutInfo> layouts;
    layouts.reserve(graph_.zones.size());

    const QFontMetrics fm(zone_label_font());
    qreal max_line_w = kMinZoneBoxWidth - (2.0 * kZonePadding);
    for (const auto& zone : graph_.zones) {
        const QStringList raw_lines = format_zone_lines(zone);
        for (const QString& line : raw_lines) {
            max_line_w = qMax(max_line_w, static_cast<qreal>(fm.horizontalAdvance(line)));
        }
    }

    qreal max_box_h = 0.0;
    for (const auto& zone : graph_.zones) {
        const ZoneLayoutInfo layout = measure_zone_layout(zone, max_line_w);
        layouts.push_back(layout);
        max_box_h = qMax(max_box_h, layout.box_size.height());
    }

    const qreal uniform_box_w = max_line_w + (2.0 * kZonePadding);
    const qreal uniform_box_h = max_box_h;
    const qreal grid_step_x = uniform_box_w + kNodeMargin;
    const qreal grid_step_y = uniform_box_h + kNodeMargin;
    const auto positions = layout_zone_nodes(graph_);

    std::vector<QRectF> node_rects;
    node_rects.reserve(layouts.size());
    std::unordered_map<int, std::size_t> layout_index_by_zone;

    for (std::size_t i = 0; i < layouts.size(); ++i) {
        const auto& layout = layouts[i];
        layout_index_by_zone[layout.zone_num] = i;

        const auto pos_it = positions.find(layout.zone_num);
        if (pos_it == positions.end()) {
            node_rects.push_back(QRectF());
            continue;
        }

        const QPointF center(pos_it->second.x * grid_step_x, pos_it->second.y * grid_step_y);
        node_rects.push_back(rect_centered_at(center, QSizeF(uniform_box_w, uniform_box_h)));
    }

    resolve_rect_overlaps(node_rects, kNodeMargin);

    std::unordered_map<int, QPointF> centers;
    for (const auto& zone : graph_.zones) {
        const auto idx_it = layout_index_by_zone.find(zone.zone_num);
        if (idx_it == layout_index_by_zone.end()) {
            continue;
        }
        const QRectF& rect = node_rects[idx_it->second];
        if (rect.isNull()) {
            continue;
        }

        auto* node_item = new ZoneNodeItem;
        node_item->zone_num = zone.zone_num;
        const bool highlighted = highlighted_zone_ == zone.zone_num;
        const QPen pen(highlighted ? QColor(220, 120, 0) : QColor(55, 55, 55), highlighted ? 3.0 : 1.0);
        const QBrush brush(highlighted ? QColor(255, 248, 220) : QColor(248, 252, 248));
        node_item->rect = scene_->addRect(rect, pen, brush);
        node_item->rect->setZValue(2);
        node_item->rect->setData(kDataZone, zone.zone_num);

        const ZoneLayoutInfo& layout = layouts[idx_it->second];
        add_zone_line_labels(scene_, rect, zone.zone_num, layout.lines);

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

        QString tip = QString("Zona %1 \u2192 %2: %3 link")
                          .arg(edge.from_zone)
                          .arg(edge.to_zone)
                          .arg(edge.link_count);
        if (edge.broken_count > 0) {
            tip += QString(", %1 rotti").arg(edge.broken_count);
        }
        line->setToolTip(tip);
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

#include "nebbie/zone_graph.hpp"

#include "nebbie/edit.hpp"

#include <algorithm>
#include <queue>
#include <sstream>

namespace nebbie {

std::vector<long> rooms_in_zone(const World& world, int zone_num) {
    std::vector<long> vnums;
    const Zone* zone = find_zone(world, zone_num);
    if (!zone) {
        return vnums;
    }

    for (const auto& [vnum, room] : world.rooms) {
        if (vnum >= zone->bottom && vnum <= zone->top) {
            vnums.push_back(vnum);
        }
    }
    std::sort(vnums.begin(), vnums.end());
    return vnums;
}

ZoneGraph build_zone_graph(const World& world, int zone_num) {
    ZoneGraph graph;
    graph.zone_num = zone_num;

    const Zone* zone = find_zone(world, zone_num);
    if (!zone) {
        return graph;
    }

    graph.zone_name = zone->name;
    graph.bottom = zone->bottom;
    graph.top = zone->top;

    const auto vnums = rooms_in_zone(world, zone_num);
    for (long vnum : vnums) {
        const Room* room = world.find_room(vnum);
        if (!room) {
            continue;
        }
        graph.nodes.push_back({vnum, room->name, static_cast<int>(room->sector_type)});
        for (const auto& exit : room->exits) {
            ZoneGraphEdge edge;
            edge.from_vnum = vnum;
            edge.to_vnum = exit.to_room;
            edge.direction = exit.direction;
            edge.broken = exit.to_room > 0 && !world.find_room(exit.to_room);
            graph.edges.push_back(edge);
        }
    }

    return graph;
}

namespace {

bool room_in_graph(const ZoneGraph& graph, long vnum) {
    for (const auto& node : graph.nodes) {
        if (node.vnum == vnum) {
            return true;
        }
    }
    return false;
}

} // namespace

ZoneZLayout compute_zone_z_levels(const ZoneGraph& graph) {
    ZoneZLayout layout;
    if (graph.nodes.empty()) {
        return layout;
    }

    long seed = graph.nodes.front().vnum;
    for (const auto& node : graph.nodes) {
        seed = std::min(seed, node.vnum);
    }

    std::queue<long> pending;
    pending.push(seed);
    layout.levels[seed] = 0;

    while (!pending.empty()) {
        const long from_vnum = pending.front();
        pending.pop();
        const int from_z = layout.levels[from_vnum];

        for (const auto& edge : graph.edges) {
            if (edge.from_vnum != from_vnum || edge.to_vnum <= 0 || edge.broken) {
                continue;
            }
            if (!room_in_graph(graph, edge.to_vnum)) {
                continue;
            }

            int target_z = from_z;
            if (edge.direction == 4) {
                target_z = from_z + 1;
            } else if (edge.direction == 5) {
                target_z = from_z - 1;
            } else if (edge.direction > 3) {
                continue;
            }

            if (!layout.levels.count(edge.to_vnum)) {
                layout.levels[edge.to_vnum] = target_z;
                pending.push(edge.to_vnum);
            }
        }
    }

    for (const auto& node : graph.nodes) {
        if (!layout.levels.count(node.vnum)) {
            layout.levels[node.vnum] = 0;
        }
    }

    layout.min_level = layout.max_level = layout.levels.begin()->second;
    for (const auto& [vnum, level] : layout.levels) {
        (void)vnum;
        layout.min_level = std::min(layout.min_level, level);
        layout.max_level = std::max(layout.max_level, level);
    }

    return layout;
}

std::vector<int> sorted_z_levels(const ZoneZLayout& layout) {
    std::vector<int> levels;
    if (layout.levels.empty()) {
        levels.push_back(0);
        return levels;
    }

    for (int level = layout.min_level; level <= layout.max_level; ++level) {
        levels.push_back(level);
    }
    return levels;
}

WorldZoneNode build_world_zone_node(const World& world, const Zone& zone) {
    WorldZoneNode node;
    node.zone_num = zone.num;
    node.name = zone.name;
    node.bottom = zone.bottom;
    node.top = zone.top;

    for (long vnum = zone.bottom; vnum <= zone.top; ++vnum) {
        if (world.find_room(vnum)) {
            node.used_vnums.push_back(vnum);
        }
    }
    node.used_count = static_cast<int>(node.used_vnums.size());

    long range_start = -1;
    for (long vnum = zone.bottom; vnum <= zone.top; ++vnum) {
        if (!world.find_room(vnum)) {
            if (range_start < 0) {
                range_start = vnum;
            }
        } else if (range_start >= 0) {
            node.free_ranges.push_back({range_start, vnum - 1});
            range_start = -1;
        }
    }
    if (range_start >= 0) {
        node.free_ranges.push_back({range_start, zone.top});
    }

    node.free_count = 0;
    for (const auto& range : node.free_ranges) {
        node.free_count += static_cast<int>(range.end - range.start + 1);
    }

    return node;
}

WorldZoneGraph build_world_zone_graph(const World& world) {
    WorldZoneGraph graph;
    for (const auto& zone : world.zones) {
        graph.zones.push_back(build_world_zone_node(world, zone));
    }

    std::map<std::pair<int, int>, WorldZoneEdge> edge_map;
    for (const auto& [vnum, room] : world.rooms) {
        const Zone* from_zone = world.zone_for_vnum(vnum);
        if (!from_zone) {
            continue;
        }

        for (const auto& exit : room.exits) {
            if (exit.to_room <= 0) {
                continue;
            }

            const Zone* to_zone = world.zone_for_vnum(exit.to_room);
            if (!to_zone || to_zone->num == from_zone->num) {
                continue;
            }

            const auto key = std::make_pair(from_zone->num, to_zone->num);
            WorldZoneEdge& edge = edge_map[key];
            if (edge.link_count == 0) {
                edge.from_zone = from_zone->num;
                edge.to_zone = to_zone->num;
                edge.sample_from_vnum = vnum;
                edge.sample_to_vnum = exit.to_room;
            }

            ++edge.link_count;
            if (!world.find_room(exit.to_room)) {
                ++edge.broken_count;
            }
        }
    }

    for (auto& [key, edge] : edge_map) {
        (void)key;
        graph.edges.push_back(std::move(edge));
    }

    return graph;
}

std::string format_vnum_ranges(const std::vector<VnumRange>& ranges, std::size_t max_ranges) {
    if (ranges.empty()) {
        return "(nessuno)";
    }

    std::ostringstream out;
    for (std::size_t i = 0; i < ranges.size() && i < max_ranges; ++i) {
        if (i > 0) {
            out << ", ";
        }
        if (ranges[i].start == ranges[i].end) {
            out << ranges[i].start;
        } else {
            out << ranges[i].start << '-' << ranges[i].end;
        }
    }
    if (ranges.size() > max_ranges) {
        out << ", ... (+" << (ranges.size() - max_ranges) << " intervalli)";
    }
    return out.str();
}

std::string zone_graph_to_dot(const ZoneGraph& graph) {
    std::ostringstream out;
    out << "digraph zone_" << graph.zone_num << " {\n";
    out << "  label=\"Zone " << graph.zone_num << ": " << graph.zone_name << "\";\n";
    out << "  node [shape=box, style=rounded];\n";

    for (const auto& node : graph.nodes) {
        out << "  r" << node.vnum << " [label=\"#" << node.vnum << "\\n"
            << node.name << "\"];\n";
    }

    for (const auto& edge : graph.edges) {
        if (edge.to_vnum <= 0) {
            continue;
        }
        out << "  r" << edge.from_vnum << " -> r" << edge.to_vnum;
        out << " [label=\"" << exit_direction_name(edge.direction) << "\"";
        if (edge.broken) {
            out << ", color=red, style=dashed";
        } else if (edge.direction >= 4) {
            out << ", style=dashed, color=blue";
        }
        out << "];\n";
    }

    out << "}\n";
    return out.str();
}

std::string world_zone_graph_to_dot(const WorldZoneGraph& graph) {
    std::ostringstream out;
    out << "digraph world_zones {\n";
    out << "  label=\"Collegamenti tra zone\";\n";
    out << "  node [shape=box, style=rounded];\n";

    for (const auto& zone : graph.zones) {
        out << "  z" << zone.zone_num << " [label=\"#" << zone.zone_num << "\\n"
            << zone.name << "\\n[" << zone.bottom << "-" << zone.top << "]\\n"
            << "usati:" << zone.used_count << " liberi:" << zone.free_count << "\"];\n";
    }

    for (const auto& edge : graph.edges) {
        out << "  z" << edge.from_zone << " -> z" << edge.to_zone;
        out << " [label=\"" << edge.link_count << " link";
        if (edge.broken_count > 0) {
            out << ", " << edge.broken_count << " rotti";
        }
        out << "\"";
        if (edge.broken_count > 0) {
            out << ", color=red, style=dashed";
        }
        out << "];\n";
    }

    out << "}\n";
    return out.str();
}

} // namespace nebbie

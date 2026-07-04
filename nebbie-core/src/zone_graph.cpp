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

} // namespace nebbie

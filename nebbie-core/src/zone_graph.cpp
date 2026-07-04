#include "nebbie/zone_graph.hpp"

#include "nebbie/edit.hpp"

#include <algorithm>
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

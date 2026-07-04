#pragma once

#include "world.hpp"

#include <string>
#include <vector>

namespace nebbie {

struct ZoneGraphNode {
    long vnum = 0;
    std::string name;
    int sector_type = 0;
};

struct ZoneGraphEdge {
    long from_vnum = 0;
    long to_vnum = 0;
    int direction = 0;
    bool broken = false;
};

struct ZoneGraph {
    int zone_num = 0;
    std::string zone_name;
    int bottom = 0;
    int top = 0;
    std::vector<ZoneGraphNode> nodes;
    std::vector<ZoneGraphEdge> edges;
};

std::vector<long> rooms_in_zone(const World& world, int zone_num);
ZoneGraph build_zone_graph(const World& world, int zone_num);
std::string zone_graph_to_dot(const ZoneGraph& graph);

} // namespace nebbie

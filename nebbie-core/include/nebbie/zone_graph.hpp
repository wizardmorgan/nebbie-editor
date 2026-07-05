#pragma once

#include "world.hpp"

#include <map>
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

struct ZoneZLayout {
    std::map<long, int> levels;
    int min_level = 0;
    int max_level = 0;
};

struct VnumRange {
    long start = 0;
    long end = 0;
};

struct WorldZoneNode {
    int zone_num = 0;
    std::string name;
    int bottom = 0;
    int top = 0;
    std::vector<long> used_vnums;
    std::vector<VnumRange> free_ranges;
    int used_count = 0;
    int free_count = 0;
};

struct WorldZoneEdge {
    int from_zone = 0;
    int to_zone = 0;
    int link_count = 0;
    int broken_count = 0;
    long sample_from_vnum = 0;
    long sample_to_vnum = 0;
};

struct WorldZoneGraph {
    std::vector<WorldZoneNode> zones;
    std::vector<WorldZoneEdge> edges;
};

std::vector<long> rooms_in_zone(const World& world, int zone_num);
ZoneGraph build_zone_graph(const World& world, int zone_num);
ZoneZLayout compute_zone_z_levels(const ZoneGraph& graph);
std::vector<int> sorted_z_levels(const ZoneZLayout& layout);
WorldZoneNode build_world_zone_node(const World& world, const Zone& zone);
WorldZoneGraph build_world_zone_graph(const World& world);
std::string zone_graph_to_dot(const ZoneGraph& graph);
std::string world_zone_graph_to_dot(const WorldZoneGraph& graph);
std::string format_vnum_ranges(const std::vector<VnumRange>& ranges, std::size_t max_ranges = 8);

} // namespace nebbie

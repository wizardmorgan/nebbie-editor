#pragma once

#include "zone_graph.hpp"

#include <optional>
#include <string>
#include <vector>

namespace nebbie {

struct WorldIndexZone {
    int table_index = 0;
    int zone_num = 0;
    std::string name;
    int bottom = 0;
    int top = 0;
    std::vector<long> rooms_used;
    std::vector<VnumRange> rooms_free;
    int rooms_used_count = 0;
    int rooms_free_count = 0;
};

struct WorldIndexReservation {
    std::string builder;
    int zone_num = 0;
    std::string kind = "room";
    long start_vnum = 0;
    long end_vnum = 0;
    std::string note;
    std::string expires_at;
    std::string status = "active";
};

struct WorldIndex {
    std::string schema_version = "1";
    std::string generated_at;
    std::string source = "local-export";
    std::vector<WorldIndexZone> zones;
    std::vector<long> mob_vnums;
    std::vector<long> object_vnums;
    std::vector<WorldIndexReservation> reservations;
};

WorldIndex build_world_index(const World& world, const std::string& source = "local-export");
std::string world_index_to_json(const WorldIndex& index);
std::optional<WorldIndex> world_index_from_json(const std::string& json);

bool room_vnum_taken(const WorldIndex& index, long vnum);
bool mob_vnum_taken(const WorldIndex& index, long vnum);
bool object_vnum_taken(const WorldIndex& index, long vnum);

std::optional<long> suggest_room_vnum_in_zone(const WorldIndex& index, int zone_num);
std::optional<long> suggest_mob_vnum(const WorldIndex& index);
std::optional<long> suggest_object_vnum(const WorldIndex& index);

void merge_reservations(WorldIndex& index, const std::vector<WorldIndexReservation>& remote);
std::optional<std::vector<WorldIndexReservation>> coordinator_reservations_from_json(const std::string& json);

} // namespace nebbie

#include "nebbie/world_index.hpp"

#include "nebbie/zone_graph.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace nebbie {

namespace {

std::string json_escape(const std::string& value) {
    std::string out;
    out.reserve(value.size() + 8);
    for (char c : value) {
        switch (c) {
        case '\\':
            out += "\\\\";
            break;
        case '"':
            out += "\\\"";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            out += c;
            break;
        }
    }
    return out;
}

std::string iso8601_now() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

void append_json_array_long(std::ostringstream& oss, const std::vector<long>& values) {
    oss << '[';
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            oss << ',';
        }
        oss << values[i];
    }
    oss << ']';
}

void skip_ws(const std::string& json, std::size_t& pos) {
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        ++pos;
    }
}

bool match_literal(const std::string& json, std::size_t& pos, const char* literal) {
    skip_ws(json, pos);
    const std::size_t len = std::strlen(literal);
    if (json.compare(pos, len, literal) != 0) {
        return false;
    }
    pos += len;
    return true;
}

std::optional<std::string> parse_json_string(const std::string& json, std::size_t& pos) {
    skip_ws(json, pos);
    if (pos >= json.size() || json[pos] != '"') {
        return std::nullopt;
    }
    ++pos;
    std::string out;
    while (pos < json.size()) {
        const char c = json[pos++];
        if (c == '"') {
            return out;
        }
        if (c == '\\' && pos < json.size()) {
            const char esc = json[pos++];
            switch (esc) {
            case '"':
            case '\\':
            case '/':
                out += esc;
                break;
            case 'n':
                out += '\n';
                break;
            case 'r':
                out += '\r';
                break;
            case 't':
                out += '\t';
                break;
            default:
                out += esc;
                break;
            }
            continue;
        }
        out += c;
    }
    return std::nullopt;
}

std::optional<long> parse_json_number(const std::string& json, std::size_t& pos) {
    skip_ws(json, pos);
    std::size_t start = pos;
    if (pos < json.size() && (json[pos] == '-' || json[pos] == '+')) {
        ++pos;
    }
    while (pos < json.size() && std::isdigit(static_cast<unsigned char>(json[pos]))) {
        ++pos;
    }
    if (start == pos) {
        return std::nullopt;
    }
    try {
        return std::stol(json.substr(start, pos - start));
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<std::vector<long>> parse_json_long_array(const std::string& json, std::size_t& pos) {
    skip_ws(json, pos);
    if (pos >= json.size() || json[pos] != '[') {
        return std::nullopt;
    }
    ++pos;
    std::vector<long> values;
    skip_ws(json, pos);
    if (pos < json.size() && json[pos] == ']') {
        ++pos;
        return values;
    }
    while (pos < json.size()) {
        const auto value = parse_json_number(json, pos);
        if (!value) {
            return std::nullopt;
        }
        values.push_back(*value);
        skip_ws(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
            continue;
        }
        if (pos < json.size() && json[pos] == ']') {
            ++pos;
            return values;
        }
        return std::nullopt;
    }
    return std::nullopt;
}

bool vnum_in_ranges(long vnum, const std::vector<VnumRange>& ranges) {
    for (const auto& range : ranges) {
        if (vnum >= range.start && vnum <= range.end) {
            return true;
        }
    }
    return false;
}

bool vnum_reserved(const WorldIndex& index, long vnum) {
    for (const auto& reservation : index.reservations) {
        if (reservation.status != "active") {
            continue;
        }
        if (vnum >= reservation.start_vnum && vnum <= reservation.end_vnum) {
            return true;
        }
    }
    return false;
}

const WorldIndexZone* find_zone(const WorldIndex& index, int zone_num) {
    for (const auto& zone : index.zones) {
        if (zone.zone_num == zone_num) {
            return &zone;
        }
    }
    return nullptr;
}

std::optional<std::vector<VnumRange>> parse_json_range_array(const std::string& json, std::size_t& pos) {
    skip_ws(json, pos);
    if (pos >= json.size() || json[pos] != '[') {
        return std::nullopt;
    }
    ++pos;
    std::vector<VnumRange> ranges;
    skip_ws(json, pos);
    if (pos < json.size() && json[pos] == ']') {
        ++pos;
        return ranges;
    }
    while (pos < json.size()) {
        skip_ws(json, pos);
        if (pos >= json.size() || json[pos] != '[') {
            return std::nullopt;
        }
        ++pos;
        const auto start = parse_json_number(json, pos);
        if (!start || !match_literal(json, pos, ",")) {
            return std::nullopt;
        }
        const auto end = parse_json_number(json, pos);
        if (!end || !match_literal(json, pos, "]")) {
            return std::nullopt;
        }
        ranges.push_back({*start, *end});
        skip_ws(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
            continue;
        }
        break;
    }
    skip_ws(json, pos);
    if (pos >= json.size() || json[pos] != ']') {
        return std::nullopt;
    }
    ++pos;
    return ranges;
}

std::optional<WorldIndexZone> parse_json_zone_object(const std::string& json, std::size_t& pos) {
    if (!match_literal(json, pos, "{")) {
        return std::nullopt;
    }

    WorldIndexZone zone;
    while (pos < json.size()) {
        skip_ws(json, pos);
        if (pos < json.size() && json[pos] == '}') {
            ++pos;
            return zone;
        }

        const auto key = parse_json_string(json, pos);
        if (!key || !match_literal(json, pos, ":")) {
            return std::nullopt;
        }

        if (*key == "table_index") {
            const auto value = parse_json_number(json, pos);
            if (!value) {
                return std::nullopt;
            }
            zone.table_index = static_cast<int>(*value);
        } else if (*key == "zone_num") {
            const auto value = parse_json_number(json, pos);
            if (!value) {
                return std::nullopt;
            }
            zone.zone_num = static_cast<int>(*value);
        } else if (*key == "name") {
            const auto value = parse_json_string(json, pos);
            if (!value) {
                return std::nullopt;
            }
            zone.name = *value;
        } else if (*key == "bottom" || *key == "top" || *key == "rooms_used_count"
                   || *key == "rooms_free_count") {
            const auto value = parse_json_number(json, pos);
            if (!value) {
                return std::nullopt;
            }
            if (*key == "bottom") {
                zone.bottom = static_cast<int>(*value);
            } else if (*key == "top") {
                zone.top = static_cast<int>(*value);
            } else if (*key == "rooms_used_count") {
                zone.rooms_used_count = static_cast<int>(*value);
            } else {
                zone.rooms_free_count = static_cast<int>(*value);
            }
        } else if (*key == "rooms_used") {
            const auto values = parse_json_long_array(json, pos);
            if (!values) {
                return std::nullopt;
            }
            zone.rooms_used = *values;
            std::sort(zone.rooms_used.begin(), zone.rooms_used.end());
        } else if (*key == "rooms_free") {
            const auto ranges = parse_json_range_array(json, pos);
            if (!ranges) {
                return std::nullopt;
            }
            zone.rooms_free = *ranges;
        } else {
            skip_ws(json, pos);
            if (pos < json.size() && json[pos] == '"') {
                (void)parse_json_string(json, pos);
            } else if (pos < json.size() && json[pos] == '[') {
                int depth = 1;
                ++pos;
                while (pos < json.size() && depth > 0) {
                    if (json[pos] == '[') {
                        ++depth;
                    } else if (json[pos] == ']') {
                        --depth;
                    }
                    ++pos;
                }
            } else {
                (void)parse_json_number(json, pos);
            }
        }

        skip_ws(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
        }
    }
    return std::nullopt;
}

std::optional<std::vector<WorldIndexZone>> parse_json_zones_array(const std::string& json, std::size_t& pos) {
    skip_ws(json, pos);
    if (pos >= json.size() || json[pos] != '[') {
        return std::nullopt;
    }
    ++pos;

    std::vector<WorldIndexZone> zones;
    skip_ws(json, pos);
    if (pos < json.size() && json[pos] == ']') {
        ++pos;
        return zones;
    }

    while (pos < json.size()) {
        const auto zone = parse_json_zone_object(json, pos);
        if (!zone) {
            return std::nullopt;
        }
        zones.push_back(*zone);
        skip_ws(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
            continue;
        }
        break;
    }

    skip_ws(json, pos);
    if (pos >= json.size() || json[pos] != ']') {
        return std::nullopt;
    }
    ++pos;
    return zones;
}

std::optional<WorldIndexReservation> parse_json_reservation_object(const std::string& json, std::size_t& pos) {
    if (!match_literal(json, pos, "{")) {
        return std::nullopt;
    }

    WorldIndexReservation reservation;
    while (pos < json.size()) {
        skip_ws(json, pos);
        if (pos < json.size() && json[pos] == '}') {
            ++pos;
            return reservation;
        }

        const auto key = parse_json_string(json, pos);
        if (!key || !match_literal(json, pos, ":")) {
            return std::nullopt;
        }

        if (*key == "builder" || *key == "kind" || *key == "note" || *key == "expires_at" || *key == "status") {
            const auto value = parse_json_string(json, pos);
            if (!value) {
                return std::nullopt;
            }
            if (*key == "builder") {
                reservation.builder = *value;
            } else if (*key == "kind") {
                reservation.kind = *value;
            } else if (*key == "note") {
                reservation.note = *value;
            } else if (*key == "expires_at") {
                reservation.expires_at = *value;
            } else {
                reservation.status = *value;
            }
        } else if (*key == "zone_num") {
            const auto value = parse_json_number(json, pos);
            if (!value) {
                return std::nullopt;
            }
            reservation.zone_num = static_cast<int>(*value);
        } else if (*key == "start_vnum" || *key == "end_vnum") {
            const auto value = parse_json_number(json, pos);
            if (!value) {
                return std::nullopt;
            }
            if (*key == "start_vnum") {
                reservation.start_vnum = *value;
            } else {
                reservation.end_vnum = *value;
            }
        } else {
            skip_ws(json, pos);
            if (pos < json.size() && json[pos] == '"') {
                (void)parse_json_string(json, pos);
            } else {
                (void)parse_json_number(json, pos);
            }
        }

        skip_ws(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
        }
    }
    return std::nullopt;
}

std::optional<std::vector<WorldIndexReservation>>
parse_json_reservations_array(const std::string& json, std::size_t& pos) {
    skip_ws(json, pos);
    if (pos >= json.size() || json[pos] != '[') {
        return std::nullopt;
    }
    ++pos;

    std::vector<WorldIndexReservation> reservations;
    skip_ws(json, pos);
    if (pos < json.size() && json[pos] == ']') {
        ++pos;
        return reservations;
    }

    while (pos < json.size()) {
        const auto reservation = parse_json_reservation_object(json, pos);
        if (!reservation) {
            return std::nullopt;
        }
        reservations.push_back(*reservation);
        skip_ws(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
            continue;
        }
        break;
    }

    skip_ws(json, pos);
    if (pos >= json.size() || json[pos] != ']') {
        return std::nullopt;
    }
    ++pos;
    return reservations;
}

} // namespace

WorldIndex build_world_index(const World& world, const std::string& source) {
    WorldIndex index;
    index.generated_at = iso8601_now();
    index.source = source;

    const WorldZoneGraph graph = build_world_zone_graph(world);
    index.zones.reserve(graph.zones.size());
    for (std::size_t i = 0; i < graph.zones.size(); ++i) {
        const auto& zone = graph.zones[i];
        WorldIndexZone entry;
        entry.table_index = static_cast<int>(i);
        entry.zone_num = zone.zone_num;
        entry.name = zone.name;
        entry.bottom = zone.bottom;
        entry.top = zone.top;
        entry.rooms_used = zone.used_vnums;
        entry.rooms_free = zone.free_ranges;
        entry.rooms_used_count = zone.used_count;
        entry.rooms_free_count = zone.free_count;
        index.zones.push_back(std::move(entry));
    }

    index.mob_vnums.reserve(world.mobiles.size());
    for (const auto& [vnum, mob] : world.mobiles) {
        (void)mob;
        index.mob_vnums.push_back(vnum);
    }
    std::sort(index.mob_vnums.begin(), index.mob_vnums.end());

    index.object_vnums.reserve(world.objects.size());
    for (const auto& [vnum, obj] : world.objects) {
        (void)obj;
        index.object_vnums.push_back(vnum);
    }
    std::sort(index.object_vnums.begin(), index.object_vnums.end());

    return index;
}

std::string world_index_to_json(const WorldIndex& index) {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"schema_version\": \"" << json_escape(index.schema_version) << "\",\n";
    oss << "  \"generated_at\": \"" << json_escape(index.generated_at) << "\",\n";
    oss << "  \"source\": \"" << json_escape(index.source) << "\",\n";

    oss << "  \"zones\": [\n";
    for (std::size_t i = 0; i < index.zones.size(); ++i) {
        const auto& zone = index.zones[i];
        if (i > 0) {
            oss << ",\n";
        }
        oss << "    {\n";
        oss << "      \"table_index\": " << zone.table_index << ",\n";
        oss << "      \"zone_num\": " << zone.zone_num << ",\n";
        oss << "      \"name\": \"" << json_escape(zone.name) << "\",\n";
        oss << "      \"bottom\": " << zone.bottom << ",\n";
        oss << "      \"top\": " << zone.top << ",\n";
        oss << "      \"rooms_used_count\": " << zone.rooms_used_count << ",\n";
        oss << "      \"rooms_free_count\": " << zone.rooms_free_count << ",\n";
        oss << "      \"rooms_used\": ";
        append_json_array_long(oss, zone.rooms_used);
        oss << ",\n";
        oss << "      \"rooms_free\": [";
        for (std::size_t r = 0; r < zone.rooms_free.size(); ++r) {
            if (r > 0) {
                oss << ", ";
            }
            oss << '[' << zone.rooms_free[r].start << ", " << zone.rooms_free[r].end << ']';
        }
        oss << "]\n";
        oss << "    }";
    }
    oss << "\n  ],\n";

    oss << "  \"mob_vnums\": ";
    append_json_array_long(oss, index.mob_vnums);
    oss << ",\n";

    oss << "  \"object_vnums\": ";
    append_json_array_long(oss, index.object_vnums);
    oss << ",\n";

    oss << "  \"reservations\": [\n";
    for (std::size_t i = 0; i < index.reservations.size(); ++i) {
        const auto& reservation = index.reservations[i];
        if (i > 0) {
            oss << ",\n";
        }
        oss << "    {\n";
        oss << "      \"builder\": \"" << json_escape(reservation.builder) << "\",\n";
        oss << "      \"zone_num\": " << reservation.zone_num << ",\n";
        oss << "      \"kind\": \"" << json_escape(reservation.kind) << "\",\n";
        oss << "      \"start_vnum\": " << reservation.start_vnum << ",\n";
        oss << "      \"end_vnum\": " << reservation.end_vnum << ",\n";
        oss << "      \"note\": \"" << json_escape(reservation.note) << "\",\n";
        oss << "      \"expires_at\": \"" << json_escape(reservation.expires_at) << "\",\n";
        oss << "      \"status\": \"" << json_escape(reservation.status) << "\"\n";
        oss << "    }";
    }
    oss << "\n  ]\n";
    oss << "}\n";
    return oss.str();
}

std::optional<WorldIndex> world_index_from_json(const std::string& json) {
    WorldIndex index;
    std::size_t pos = 0;
    if (!match_literal(json, pos, "{")) {
        return std::nullopt;
    }

    while (pos < json.size()) {
        skip_ws(json, pos);
        if (pos < json.size() && json[pos] == '}') {
            ++pos;
            return index;
        }

        const auto key = parse_json_string(json, pos);
        if (!key) {
            return std::nullopt;
        }
        if (!match_literal(json, pos, ":")) {
            return std::nullopt;
        }

        if (*key == "schema_version") {
            const auto value = parse_json_string(json, pos);
            if (!value) {
                return std::nullopt;
            }
            index.schema_version = *value;
        } else if (*key == "generated_at") {
            const auto value = parse_json_string(json, pos);
            if (!value) {
                return std::nullopt;
            }
            index.generated_at = *value;
        } else if (*key == "source") {
            const auto value = parse_json_string(json, pos);
            if (!value) {
                return std::nullopt;
            }
            index.source = *value;
        } else if (*key == "mob_vnums") {
            const auto values = parse_json_long_array(json, pos);
            if (!values) {
                return std::nullopt;
            }
            index.mob_vnums = *values;
        } else if (*key == "object_vnums") {
            const auto values = parse_json_long_array(json, pos);
            if (!values) {
                return std::nullopt;
            }
            index.object_vnums = *values;
        } else if (*key == "zones") {
            const auto zones = parse_json_zones_array(json, pos);
            if (!zones) {
                return std::nullopt;
            }
            index.zones = *zones;
        } else if (*key == "reservations") {
            const auto reservations = parse_json_reservations_array(json, pos);
            if (!reservations) {
                return std::nullopt;
            }
            index.reservations = *reservations;
        } else {
            skip_ws(json, pos);
            if (pos < json.size() && json[pos] == '"') {
                (void)parse_json_string(json, pos);
            } else if (pos < json.size() && json[pos] == '[') {
                int depth = 1;
                ++pos;
                while (pos < json.size() && depth > 0) {
                    if (json[pos] == '[') {
                        ++depth;
                    } else if (json[pos] == ']') {
                        --depth;
                    }
                    ++pos;
                }
            } else {
                (void)parse_json_number(json, pos);
            }
        }

        skip_ws(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
        }
    }

    return index;
}

bool room_vnum_taken(const WorldIndex& index, long vnum) {
    if (vnum_reserved(index, vnum)) {
        return true;
    }
    for (const auto& zone : index.zones) {
        if (std::binary_search(zone.rooms_used.begin(), zone.rooms_used.end(), vnum)) {
            return true;
        }
    }
    return false;
}

bool mob_vnum_taken(const WorldIndex& index, long vnum) {
    return vnum_reserved(index, vnum)
           || std::binary_search(index.mob_vnums.begin(), index.mob_vnums.end(), vnum);
}

bool object_vnum_taken(const WorldIndex& index, long vnum) {
    return vnum_reserved(index, vnum)
           || std::binary_search(index.object_vnums.begin(), index.object_vnums.end(), vnum);
}

std::optional<long> suggest_room_vnum_in_zone(const WorldIndex& index, int zone_num) {
    const WorldIndexZone* zone = find_zone(index, zone_num);
    if (!zone) {
        return std::nullopt;
    }

    for (const auto& range : zone->rooms_free) {
        for (long vnum = range.start; vnum <= range.end; ++vnum) {
            if (!room_vnum_taken(index, vnum)) {
                return vnum;
            }
        }
    }
    return std::nullopt;
}

std::optional<long> suggest_mob_vnum(const WorldIndex& index) {
    long candidate = index.mob_vnums.empty() ? 1 : index.mob_vnums.back() + 1;
    for (int attempt = 0; attempt < 100000; ++attempt, ++candidate) {
        if (!mob_vnum_taken(index, candidate)) {
            return candidate;
        }
    }
    return std::nullopt;
}

std::optional<long> suggest_object_vnum(const WorldIndex& index) {
    long candidate = index.object_vnums.empty() ? 1 : index.object_vnums.back() + 1;
    for (int attempt = 0; attempt < 100000; ++attempt, ++candidate) {
        if (!object_vnum_taken(index, candidate)) {
            return candidate;
        }
    }
    return std::nullopt;
}

void merge_reservations(WorldIndex& index, const std::vector<WorldIndexReservation>& remote) {
    index.reservations = remote;
}

std::optional<std::vector<WorldIndexReservation>> coordinator_reservations_from_json(const std::string& json) {
    std::size_t pos = 0;
    if (!match_literal(json, pos, "{")) {
        return std::nullopt;
    }

    while (pos < json.size()) {
        skip_ws(json, pos);
        if (pos < json.size() && json[pos] == '}') {
            return std::vector<WorldIndexReservation>{};
        }

        const auto key = parse_json_string(json, pos);
        if (!key || !match_literal(json, pos, ":")) {
            return std::nullopt;
        }

        if (*key == "reservations") {
            return parse_json_reservations_array(json, pos);
        }

        skip_ws(json, pos);
        if (pos < json.size() && json[pos] == '"') {
            (void)parse_json_string(json, pos);
        } else if (pos < json.size() && json[pos] == '[') {
            int depth = 1;
            ++pos;
            while (pos < json.size() && depth > 0) {
                if (json[pos] == '[') {
                    ++depth;
                } else if (json[pos] == ']') {
                    --depth;
                }
                ++pos;
            }
        } else if (pos < json.size() && json[pos] == '{') {
            int depth = 1;
            ++pos;
            while (pos < json.size() && depth > 0) {
                if (json[pos] == '{') {
                    ++depth;
                } else if (json[pos] == '}') {
                    --depth;
                }
                ++pos;
            }
        } else {
            (void)parse_json_number(json, pos);
        }

        skip_ws(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
        }
    }

    return std::nullopt;
}

} // namespace nebbie

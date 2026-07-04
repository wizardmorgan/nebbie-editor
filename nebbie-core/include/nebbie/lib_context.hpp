#pragma once

#include <filesystem>
#include <vector>

namespace nebbie {

struct LibContext {
    std::filesystem::path root;
    bool has_zon = false;
    bool has_wld = false;
    bool has_mob = false;
    bool has_obj = false;
    bool has_shp = false;
    bool has_spe = false;
    bool has_dam = false;
    bool has_act = false;
    bool has_pos = false;
    bool has_gui = false;

    std::vector<int> zone_overlay_indices;
    std::vector<long> room_overlay_vnums;
    std::vector<long> object_overlay_vnums;

    bool has_any() const;
    bool has_overlays() const;
};

} // namespace nebbie

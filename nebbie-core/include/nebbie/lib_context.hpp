#pragma once

#include <filesystem>

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

    bool has_any() const;
};

} // namespace nebbie

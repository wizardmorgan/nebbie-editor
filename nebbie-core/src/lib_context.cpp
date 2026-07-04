#include "nebbie/lib_context.hpp"

namespace nebbie {

bool LibContext::has_any() const {
    return has_zon || has_wld || has_mob || has_obj || has_shp || has_spe || has_dam || has_act
           || has_pos || has_gui;
}

bool LibContext::has_overlays() const {
    return !zone_overlay_indices.empty() || !room_overlay_vnums.empty() || !object_overlay_vnums.empty();
}

} // namespace nebbie

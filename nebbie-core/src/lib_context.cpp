#include "nebbie/lib_context.hpp"

namespace nebbie {

bool LibContext::has_any() const {
    return has_zon || has_wld || has_mob || has_obj || has_shp || has_spe || has_dam || has_act
           || has_pos || has_gui;
}

} // namespace nebbie

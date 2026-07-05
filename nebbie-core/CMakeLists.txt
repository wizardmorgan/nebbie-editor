add_library(nebbie-core
    src/fread.cpp
    src/world.cpp
    src/zon_io.cpp
    src/wld_io.cpp
    src/mob_io.cpp
    src/obj_io.cpp
    src/shp_io.cpp
    src/spe_io.cpp
    src/dam_io.cpp
    src/act_io.cpp
    src/pos_io.cpp
    src/gui_io.cpp
    src/lib_io.cpp
    src/lib_context.cpp
    src/validate.cpp
    src/edit.cpp
    src/session.cpp
    src/zone_graph.cpp
    src/mob_catalog.cpp
    src/obj_catalog.cpp
    src/room_catalog.cpp
    src/zone_catalog.cpp
)

target_include_directories(nebbie-core PUBLIC include)
target_compile_features(nebbie-core PUBLIC cxx_std_17)

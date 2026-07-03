find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(nebbieedit
    main.cpp
    main_window.cpp
    main_window.hpp
)

target_link_libraries(nebbieedit PRIVATE Qt6::Widgets nebbie-core)
target_compile_features(nebbieedit PRIVATE cxx_std_17)

if(APPLE AND NEBBIE_MACOS_BUNDLE)
    set_target_properties(nebbieedit PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.nebbie.editor"
        MACOSX_BUNDLE_BUNDLE_NAME "Nebbie Editor"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    )
endif()

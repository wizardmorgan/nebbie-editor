find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

set(NEBBIE_APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icons/nebbieedit.icns")

add_executable(nebbieedit
    main.cpp
    main_window.cpp
    main_window.hpp
    app_config.cpp
    app_config.hpp
    world_zone_map_widget.cpp
    world_zone_map_widget.hpp
    zone_map_widget.cpp
    zone_map_widget.hpp
    nebbieedit.qrc
)

if(APPLE AND EXISTS "${NEBBIE_APP_ICON}")
    set_source_files_properties("${NEBBIE_APP_ICON}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(nebbieedit PRIVATE "${NEBBIE_APP_ICON}")
endif()

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
    if(EXISTS "${NEBBIE_APP_ICON}")
        set_target_properties(nebbieedit PROPERTIES MACOSX_BUNDLE_ICON_FILE nebbieedit)
    endif()
endif()

include(GNUInstallDirs)

if(NOT (APPLE AND NEBBIE_MACOS_BUNDLE))
    install(TARGETS nebbieedit RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/nebbieedit.desktop"
            DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
endif()

if(APPLE AND NEBBIE_MACOS_BUNDLE)
    install(TARGETS nebbieedit BUNDLE DESTINATION "/Applications")
endif()

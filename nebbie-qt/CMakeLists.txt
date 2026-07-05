find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

set(CMAKE_AUTOMOC ON)

set(NEBBIE_APP_ICON_MAC "${CMAKE_CURRENT_SOURCE_DIR}/icons/nebbieedit.icns")
set(NEBBIE_APP_ICON_WIN "${CMAKE_CURRENT_SOURCE_DIR}/icons/nebbieedit.ico")

add_executable(nebbieedit
    main.cpp
    main_window.cpp
    main_window.hpp
    app_config.cpp
    app_config.hpp
    coordinator_client.cpp
    coordinator_client.hpp
    flag_group_widget.cpp
    flag_group_widget.hpp
    mob_editor_widget.cpp
    mob_editor_widget.hpp
    room_editor_widget.cpp
    room_editor_widget.hpp
    obj_editor_widget.cpp
    obj_editor_widget.hpp
    zone_editor_widget.cpp
    zone_editor_widget.hpp
    world_zone_map_widget.cpp
    world_zone_map_widget.hpp
    zone_map_widget.cpp
    zone_map_widget.hpp
    nebbieedit.qrc
)

if(APPLE AND EXISTS "${NEBBIE_APP_ICON_MAC}")
    set_source_files_properties("${NEBBIE_APP_ICON_MAC}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(nebbieedit PRIVATE "${NEBBIE_APP_ICON_MAC}")
endif()

if(WIN32 AND EXISTS "${NEBBIE_APP_ICON_WIN}")
    target_sources(nebbieedit PRIVATE "${NEBBIE_APP_ICON_WIN}")
endif()

target_link_libraries(nebbieedit PRIVATE Qt6::Widgets Qt6::Network nebbie-core)
target_compile_features(nebbieedit PRIVATE cxx_std_17)

if(WIN32)
    set_target_properties(nebbieedit PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(APPLE AND NEBBIE_MACOS_BUNDLE)
    set_target_properties(nebbieedit PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.nebbie.editor"
        MACOSX_BUNDLE_BUNDLE_NAME "Nebbie Editor"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    )
    if(EXISTS "${NEBBIE_APP_ICON_MAC}")
        set_target_properties(nebbieedit PROPERTIES MACOSX_BUNDLE_ICON_FILE nebbieedit)
    endif()
endif()

include(GNUInstallDirs)

if(NOT (APPLE AND NEBBIE_MACOS_BUNDLE))
    install(TARGETS nebbieedit RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    if(NOT WIN32)
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/nebbieedit.desktop"
                DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    endif()
endif()

if(APPLE AND NEBBIE_MACOS_BUNDLE)
    install(TARGETS nebbieedit BUNDLE DESTINATION "/Applications")
endif()

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${Qt6_DIR}/../../../bin" "${_qt_bin_dir}")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET nebbieedit POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:nebbieedit>"
            COMMENT "Deploying Qt runtime for nebbieedit"
        )
    endif()
endif()

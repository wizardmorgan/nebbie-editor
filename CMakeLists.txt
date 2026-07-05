cmake_minimum_required(VERSION 3.16)
project(nebbie-editor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(NEBBIE_BUILD_QT "Build Qt GUI (nebbieedit)" ON)
option(NEBBIE_MACOS_BUNDLE "Build nebbieedit as a macOS .app bundle" OFF)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum supported macOS version")
endif()

if(APPLE AND NEBBIE_BUILD_QT)
    find_program(_nebbie_brew brew)
    if(_nebbie_brew)
        execute_process(
            COMMAND "${_nebbie_brew}" --prefix qt@6
            OUTPUT_VARIABLE _nebbie_brew_qt6
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(_nebbie_brew_qt6 AND EXISTS "${_nebbie_brew_qt6}/lib/cmake/Qt6")
            list(PREPEND CMAKE_PREFIX_PATH "${_nebbie_brew_qt6}")
            message(STATUS "Using Homebrew Qt 6 at ${_nebbie_brew_qt6}")
        endif()
    endif()
endif()

if(MSVC)
    add_compile_options(/utf-8 /EHsc)
endif()

add_subdirectory(nebbie-core)
add_subdirectory(nebbiedit)

if(NEBBIE_BUILD_QT)
    find_package(Qt6 QUIET COMPONENTS Widgets)
    if(Qt6_FOUND)
        add_subdirectory(nebbie-qt)
    else()
        message(STATUS "Qt6 not found — nebbie-qt GUI will not be built")
        message(STATUS "  Linux: sudo apt install qt6-base-dev")
        message(STATUS "  macOS: brew install qt@6 && export CMAKE_PREFIX_PATH=\"$(brew --prefix qt@6)\"")
        message(STATUS "  Windows: install Qt 6 (MSVC) and set CMAKE_PREFIX_PATH to the kit root")
    endif()
endif()

enable_testing()
add_subdirectory(tests)

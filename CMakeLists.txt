cmake_minimum_required(VERSION 3.16)
project(nebbie-editor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(NEBBIE_BUILD_QT "Build Qt GUI (nebbieedit)" ON)

add_subdirectory(nebbie-core)
add_subdirectory(nebbiedit)

if(NEBBIE_BUILD_QT)
    find_package(Qt6 QUIET COMPONENTS Widgets)
    if(Qt6_FOUND)
        add_subdirectory(nebbie-qt)
    else()
        message(STATUS "Qt6 not found — nebbie-qt GUI will not be built")
    endif()
endif()

enable_testing()
add_subdirectory(tests)

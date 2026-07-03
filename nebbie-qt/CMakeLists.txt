find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(nebbieedit
    main.cpp
    main_window.cpp
    main_window.hpp
)

target_link_libraries(nebbieedit PRIVATE Qt6::Widgets nebbie-core)
target_compile_features(nebbieedit PRIVATE cxx_std_17)

add_executable(nebbiedit main.cpp cli_parse.cpp shell.cpp)
target_link_libraries(nebbiedit PRIVATE nebbie-core)

if(MSVC)
    target_compile_options(nebbiedit PRIVATE "/FI${CMAKE_SOURCE_DIR}/nebbie-qt/msvc_qt_compat.hpp")
endif()

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(nebbiedit PRIVATE -municode)
endif()

include(GNUInstallDirs)
install(TARGETS nebbiedit RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

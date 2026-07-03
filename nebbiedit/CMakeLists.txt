add_executable(nebbiedit main.cpp cli_parse.cpp shell.cpp)
target_link_libraries(nebbiedit PRIVATE nebbie-core)

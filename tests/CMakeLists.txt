add_executable(nebbie-core-tests test_io.cpp)
target_link_libraries(nebbie-core-tests PRIVATE nebbie-core)

add_test(NAME load-fixtures COMMAND nebbie-core-tests ${CMAKE_SOURCE_DIR}/tests/fixtures)

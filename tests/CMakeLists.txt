add_executable(nebbie-core-tests test_io.cpp)
target_link_libraries(nebbie-core-tests PRIVATE nebbie-core)

add_executable(nebbie-validate-tests test_validate.cpp)
target_link_libraries(nebbie-validate-tests PRIVATE nebbie-core)

add_executable(nebbie-phase4-tests test_phase4.cpp)
target_link_libraries(nebbie-phase4-tests PRIVATE nebbie-core)

add_test(NAME load-fixtures COMMAND nebbie-core-tests ${CMAKE_SOURCE_DIR}/tests/fixtures)
add_test(NAME validate-fixtures COMMAND nebbie-validate-tests ${CMAKE_SOURCE_DIR}/tests/fixtures)

if(EXISTS "${CMAKE_SOURCE_DIR}/vendor/nebbietest/mudroot/lib/myst.act")
    add_test(NAME nebbietest-phase4 COMMAND nebbie-phase4-tests
             ${CMAKE_SOURCE_DIR}/vendor/nebbietest/mudroot/lib)
endif()

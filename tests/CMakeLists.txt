add_executable(nebbie-core-tests test_io.cpp)
target_link_libraries(nebbie-core-tests PRIVATE nebbie-core)

add_executable(nebbie-validate-tests test_validate.cpp)
target_link_libraries(nebbie-validate-tests PRIVATE nebbie-core)

add_executable(nebbie-phase4-tests test_phase4.cpp)
target_link_libraries(nebbie-phase4-tests PRIVATE nebbie-core)

add_executable(nebbie-mvp-tests test_mvp.cpp)
target_link_libraries(nebbie-mvp-tests PRIVATE nebbie-core)

add_executable(nebbie-create-tests test_create.cpp)
target_link_libraries(nebbie-create-tests PRIVATE nebbie-core)

add_test(NAME load-fixtures COMMAND nebbie-core-tests ${CMAKE_SOURCE_DIR}/tests/fixtures)
add_test(NAME validate-fixtures COMMAND nebbie-validate-tests ${CMAKE_SOURCE_DIR}/tests/fixtures)

add_test(NAME mvp-edit-workflow
         COMMAND nebbie-mvp-tests ${CMAKE_SOURCE_DIR}/tests/fixtures
                                  ${CMAKE_BINARY_DIR}/mvp-work-lib)

add_test(NAME create-edit-workflow
         COMMAND nebbie-create-tests ${CMAKE_SOURCE_DIR}/tests/fixtures
                                       ${CMAKE_BINARY_DIR}/create-work-lib)

add_executable(nebbie-reset-tests test_reset.cpp)
target_link_libraries(nebbie-reset-tests PRIVATE nebbie-core)

add_executable(nebbie-session-tests test_session.cpp)
target_link_libraries(nebbie-session-tests PRIVATE nebbie-core)

add_test(NAME session-workflow COMMAND nebbie-session-tests ${CMAKE_SOURCE_DIR}/tests/fixtures)

add_test(NAME zone-reset-workflow
         COMMAND nebbie-reset-tests ${CMAKE_SOURCE_DIR}/tests/fixtures
                                    ${CMAKE_BINARY_DIR}/reset-work-lib)

add_executable(nebbie-world-index-tests test_world_index.cpp)
target_link_libraries(nebbie-world-index-tests PRIVATE nebbie-core)
add_test(NAME world-index-fixtures
         COMMAND nebbie-world-index-tests ${CMAKE_SOURCE_DIR}/tests/fixtures)

add_executable(nebbie-nebbietest-mob-tests test_nebbietest_mob.cpp)
target_link_libraries(nebbie-nebbietest-mob-tests PRIVATE nebbie-core)

if(EXISTS "${CMAKE_SOURCE_DIR}/vendor/nebbietest/mudroot/lib/myst.mob")
    set(NEBBIETEST_MOB "${CMAKE_SOURCE_DIR}/vendor/nebbietest/mudroot/lib/myst.mob")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/vendor/nebbietest/myst.mob")
    set(NEBBIETEST_MOB "${CMAKE_SOURCE_DIR}/vendor/nebbietest/myst.mob")
endif()
if(NEBBIETEST_MOB)
    add_test(NAME nebbietest-mob-load COMMAND nebbie-nebbietest-mob-tests ${NEBBIETEST_MOB})
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/vendor/nebbietest/mudroot/lib/myst.mob")
    add_test(NAME nebbietest-lib-load COMMAND nebbie-nebbietest-mob-tests
             --lib ${CMAKE_SOURCE_DIR}/vendor/nebbietest/mudroot/lib)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/vendor/nebbietest/mudroot/lib/myst.act")
    add_test(NAME nebbietest-phase4 COMMAND nebbie-phase4-tests
             ${CMAKE_SOURCE_DIR}/vendor/nebbietest/mudroot/lib)
endif()

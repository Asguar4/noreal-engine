include(FetchContent)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(googletest)

add_executable(riz_tests
  file_test.cpp
  camera_test.cpp
  input_test.cpp)
target_link_libraries(riz_tests PRIVATE riz_engine GTest::gtest_main)
target_compile_definitions(riz_tests PRIVATE RIZ_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

include(GoogleTest)
gtest_discover_tests(riz_tests WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}" DISCOVERY_MODE PRE_TEST)

add_executable(riz_gl_tests gl_test.cpp platform_gl_test.cpp)
target_link_libraries(riz_gl_tests PRIVATE riz_engine GTest::gtest_main)
target_compile_definitions(riz_gl_tests PRIVATE RIZ_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
gtest_discover_tests(riz_gl_tests WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}" DISCOVERY_MODE PRE_TEST PROPERTIES LABELS gl)

add_executable(riz_bsp_tests bsp_geometry_test.cpp bsp_build_test.cpp bsp_query_test.cpp)
target_link_libraries(riz_bsp_tests PRIVATE riz_bsp GTest::gtest_main)
gtest_discover_tests(riz_bsp_tests WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}" DISCOVERY_MODE PRE_TEST)

add_test(NAME game_smoke
         COMMAND $<TARGET_FILE:game> --smoke-frames 120
         WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
set_tests_properties(game_smoke PROPERTIES LABELS gl TIMEOUT 60)

add_test(NAME riz_layering
         COMMAND ${CMAKE_COMMAND} -DRIZ_SOURCE_DIR=${PROJECT_SOURCE_DIR} -P ${PROJECT_SOURCE_DIR}/cmake/check_layering.cmake)
add_test(NAME riz_layering_selftest
         COMMAND ${CMAKE_COMMAND} -DRIZ_SOURCE_DIR=${PROJECT_SOURCE_DIR}/tests/layering_fixture -P ${PROJECT_SOURCE_DIR}/cmake/check_layering.cmake)
set_tests_properties(riz_layering_selftest PROPERTIES
  PASS_REGULAR_EXPRESSION "riz_layering: 4 violation\\(s\\); rules violated: R1,R2,R3,R4")

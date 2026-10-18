include(FetchContent)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(googletest)

add_executable(riz_tests
  file_test.cpp
  top_down_camera_test.cpp
  input_test.cpp
  platform_test.cpp
  loop_test.cpp
  movement_test.cpp)
target_link_libraries(riz_tests PRIVATE riz_engine riz_glfw GTest::gtest_main)
riz_stage_headers(RIZ_TESTS_WHITEBOX_DIR riz_tests_whitebox engine/platform/iPlatform.hpp)
target_include_directories(riz_tests PRIVATE "${RIZ_TESTS_WHITEBOX_DIR}")
target_compile_definitions(riz_tests PRIVATE RIZ_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

include(GoogleTest)
gtest_discover_tests(riz_tests DISCOVERY_MODE PRE_TEST)

add_executable(riz_gl_tests render_gl_test.cpp platform_gl_test.cpp)
target_link_libraries(riz_gl_tests PRIVATE riz_engine riz_glew riz_glfw GTest::gtest_main)
riz_stage_headers(RIZ_GL_TESTS_WHITEBOX_DIR riz_gl_tests_whitebox engine/render/iRender.hpp)
target_include_directories(riz_gl_tests PRIVATE "${RIZ_GL_TESTS_WHITEBOX_DIR}")
target_compile_definitions(riz_gl_tests PRIVATE RIZ_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
gtest_discover_tests(riz_gl_tests DISCOVERY_MODE PRE_TEST PROPERTIES LABELS gl)

add_executable(riz_bsp_tests bsp_geometry_test.cpp bsp_build_test.cpp bsp_query_test.cpp)
target_link_libraries(riz_bsp_tests PRIVATE riz_bsp GTest::gtest_main)
riz_stage_headers(RIZ_BSP_TESTS_WHITEBOX_DIR riz_bsp_tests_whitebox containers/bsp/iBsp.hpp)
target_include_directories(riz_bsp_tests PRIVATE "${RIZ_BSP_TESTS_WHITEBOX_DIR}")
gtest_discover_tests(riz_bsp_tests DISCOVERY_MODE PRE_TEST)

add_executable(riz_game_tests bsp_level_test.cpp player_test.cpp automap_test.cpp)
target_link_libraries(riz_game_tests PRIVATE riz_game GTest::gtest_main)
riz_stage_headers(RIZ_GAME_TESTS_WHITEBOX_DIR riz_game_tests_whitebox containers/bsp/iBsp.hpp)
target_include_directories(riz_game_tests PRIVATE "${RIZ_GAME_TESTS_WHITEBOX_DIR}")
gtest_discover_tests(riz_game_tests DISCOVERY_MODE PRE_TEST)

add_test(NAME game_smoke
         COMMAND $<TARGET_FILE:game> --smoke-frames 120
         WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
set_tests_properties(game_smoke PROPERTIES LABELS gl TIMEOUT 60)

add_test(NAME riz_layering
         COMMAND ${CMAKE_COMMAND} -DRIZ_SOURCE_DIR=${PROJECT_SOURCE_DIR} -P ${PROJECT_SOURCE_DIR}/cmake/check_layering.cmake)
add_test(NAME riz_layering_selftest
         COMMAND ${CMAKE_COMMAND} -DRIZ_SOURCE_DIR=${PROJECT_SOURCE_DIR}/tests/layering_fixture -P ${PROJECT_SOURCE_DIR}/cmake/check_layering.cmake)
set_tests_properties(riz_layering_selftest PROPERTIES
  PASS_REGULAR_EXPRESSION "riz_layering: 8 violation\\(s\\); rules violated: R1,R2,R4,R5,R6")

add_test(NAME riz_include_surface
         COMMAND ${CMAKE_COMMAND} -DRIZ_SOURCE_DIR=${PROJECT_SOURCE_DIR} -DRIZ_SURFACE_FILE=${PROJECT_BINARY_DIR}/riz_include_surface.cmake
                 -P ${PROJECT_SOURCE_DIR}/cmake/check_include_surface.cmake)
add_test(NAME riz_include_surface_selftest
         COMMAND ${CMAKE_COMMAND} -DRIZ_SOURCE_DIR=${PROJECT_SOURCE_DIR}
                 -DRIZ_ENGINE_DIRS=${PROJECT_SOURCE_DIR}/src -DRIZ_BSP_DIRS=${PROJECT_SOURCE_DIR}/src
                 -P ${PROJECT_SOURCE_DIR}/cmake/check_include_surface.cmake)
set_tests_properties(riz_include_surface_selftest PROPERTIES
  PASS_REGULAR_EXPRESSION "checks violated: S1,S2,S3")

include(FetchContent)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(googletest)

add_executable(riz_tests
  smoke_test.cpp
  vector_test.cpp
  file_test.cpp
  camera_test.cpp)
target_link_libraries(riz_tests PRIVATE riz_engine GTest::gtest_main)
target_compile_definitions(riz_tests PRIVATE RIZ_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

include(GoogleTest)
gtest_discover_tests(riz_tests WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}" DISCOVERY_MODE PRE_TEST)

add_executable(riz_gl_tests gl_test.cpp)
target_link_libraries(riz_gl_tests PRIVATE riz_engine GTest::gtest_main)
target_compile_definitions(riz_gl_tests PRIVATE RIZ_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
gtest_discover_tests(riz_gl_tests WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}" DISCOVERY_MODE PRE_TEST PROPERTIES LABELS gl)

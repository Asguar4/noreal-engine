cmake_minimum_required(VERSION 3.16)
project(riz LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT MSVC)
  message(FATAL_ERROR "The vendored GLEW/GLFW libraries in lib/ are MSVC x64 .lib files: use MSVC.")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "The vendored GLEW/GLFW libraries in lib/ are x64 only: configure with -A x64.")
endif()

# Static CRT (/MT, /MTd) to match the vendored glfw3_mt.lib. Must be set before FetchContent so googletest matches.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(RIZ_DEPS_DIR "${PROJECT_SOURCE_DIR}" CACHE PATH "Directory containing include/ and lib/ with GLEW, GLFW and GLM")

foreach(_lib glew32s.lib glfw3_mt.lib)
  if(NOT EXISTS "${RIZ_DEPS_DIR}/lib/${_lib}")
    message(FATAL_ERROR "Missing ${RIZ_DEPS_DIR}/lib/${_lib}. Put the vendored dependencies into include/ and lib/ "
                        "or set -DRIZ_DEPS_DIR=<dir containing include/ and lib/>.")
  endif()
endforeach()

find_package(OpenGL REQUIRED)

add_library(riz_glm INTERFACE)
target_include_directories(riz_glm INTERFACE "${RIZ_DEPS_DIR}/include")

add_library(riz_glew STATIC IMPORTED)
set_target_properties(riz_glew PROPERTIES
  IMPORTED_LOCATION "${RIZ_DEPS_DIR}/lib/glew32s.lib"
  INTERFACE_COMPILE_DEFINITIONS GLEW_STATIC
  INTERFACE_INCLUDE_DIRECTORIES "${RIZ_DEPS_DIR}/include"
  INTERFACE_LINK_LIBRARIES OpenGL::GL)

add_library(riz_glfw STATIC IMPORTED)
set_target_properties(riz_glfw PROPERTIES
  IMPORTED_LOCATION "${RIZ_DEPS_DIR}/lib/glfw3_mt.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${RIZ_DEPS_DIR}/include"
  INTERFACE_LINK_LIBRARIES "user32;gdi32;shell32")

add_library(riz_engine STATIC
  src/engine/io/io.cpp
  src/engine/camera/camera.cpp
  src/engine/camera/topDownCamera.cpp
  src/engine/input/input.cpp
  src/engine/platform/platform.cpp
  src/engine/render/render.cpp
  src/engine/render/renderBatch.cpp)
target_include_directories(riz_engine PUBLIC src)
target_link_libraries(riz_engine PUBLIC riz_glew riz_glfw riz_glm OpenGL::GL)
target_compile_options(riz_engine PRIVATE /W4 /permissive-)

add_library(riz_bsp STATIC
  src/containers/bsp/geometry.cpp
  src/containers/bsp/build.cpp
  src/containers/bsp/query.cpp
  src/containers/bsp/invariants.cpp)
target_include_directories(riz_bsp PUBLIC src)
target_link_libraries(riz_bsp PUBLIC riz_glm)
target_compile_options(riz_bsp PRIVATE /W4 /permissive-)

add_executable(game src/main.cpp)
set_target_properties(game PROPERTIES
  OUTPUT_NAME game
  VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
target_link_libraries(game PRIVATE riz_engine)
target_compile_options(game PRIVATE /W4 /permissive-)

option(RIZ_BUILD_TESTS "Build tests" ON)
if(RIZ_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

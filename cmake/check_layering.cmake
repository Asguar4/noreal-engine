# Include-rule check for the engine layering (design 10.3.6, enforces debt 28).
# Usage: cmake -DRIZ_SOURCE_DIR=<root> -P cmake/check_layering.cmake
#
# Rules (paths relative to <root>):
#   R1  GLFW include outside src/engine/platform/
#   R2  GL include outside src/engine/render.hpp and src/engine/render/
#   R3  identifiers keyCallback / mouseCallback anywhere in src/
#   R4  forbidden cross-module includes (platform.hpp from render/input/entity/io,
#       render.hpp or iRender.hpp from platform/input/entity/io)

if(NOT RIZ_SOURCE_DIR)
  message(FATAL_ERROR "riz_layering: pass -DRIZ_SOURCE_DIR=<root>")
endif()

file(GLOB_RECURSE _files "${RIZ_SOURCE_DIR}/src/*.hpp" "${RIZ_SOURCE_DIR}/src/*.cpp")

set(_count 0)
set(_rules "")

macro(_violation rule rel reason)
  message("LAYERING ${rule}: ${rel}: ${reason}")
  math(EXPR _count "${_count} + 1")
  list(APPEND _rules "${rule}")
endmacro()

foreach(_file IN LISTS _files)
  file(RELATIVE_PATH _rel "${RIZ_SOURCE_DIR}" "${_file}")
  file(READ "${_file}" _content)

  # R3
  if(_content MATCHES "keyCallback|mouseCallback")
    _violation(R3 "${_rel}" "uses keyCallback/mouseCallback")
  endif()

  set(_inPlatform FALSE)
  set(_inRender FALSE)
  if(_rel MATCHES "^src/engine/platform[./]")
    set(_inPlatform TRUE)
  endif()
  if(_rel MATCHES "^src/engine/render[./]")
    set(_inRender TRUE)
  endif()
  # modules that must not see platform.hpp
  set(_noPlatform FALSE)
  if(_rel MATCHES "^src/engine/(render|input|entity|io)[./]")
    set(_noPlatform TRUE)
  endif()
  # modules that must not see render.hpp / iRender.hpp
  set(_noRender FALSE)
  if(_rel MATCHES "^src/engine/(platform|input|entity|io)[./]")
    set(_noRender TRUE)
  endif()

  string(REGEX MATCHALL "(^|\n)[ \t]*#[ \t]*include[ \t]*[<\"][^>\"\n]+[>\"]" _includes "${_content}")
  foreach(_inc IN LISTS _includes)
    string(REGEX REPLACE "^.*#[ \t]*include[ \t]*[<\"]([^>\"\n]+)[>\"]$" "\\1" _path "${_inc}")
    get_filename_component(_name "${_path}" NAME)

    # R1
    if(_path MATCHES "^GLFW/" AND NOT _inPlatform)
      _violation(R1 "${_rel}" "includes ${_path} outside src/engine/platform/")
    endif()
    # R2
    if(_path MATCHES "^GL/" AND NOT _rel STREQUAL "src/engine/render.hpp" AND NOT _inRender)
      _violation(R2 "${_rel}" "includes ${_path} outside the render module")
    endif()
    # R4
    if(_noPlatform AND _name STREQUAL "platform.hpp")
      _violation(R4 "${_rel}" "includes platform.hpp from a module that must not depend on platform")
    endif()
    if(_noRender AND (_name STREQUAL "render.hpp" OR _name STREQUAL "iRender.hpp"))
      _violation(R4 "${_rel}" "includes ${_name} from a module that must not depend on render")
    endif()
  endforeach()
endforeach()

if(_count GREATER 0)
  list(REMOVE_DUPLICATES _rules)
  list(SORT _rules)
  string(REPLACE ";" "," _rulesText "${_rules}")
  message(FATAL_ERROR "riz_layering: ${_count} violation(s); rules violated: ${_rulesText}")
endif()

message("riz_layering: OK")

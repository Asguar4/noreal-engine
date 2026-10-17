# Include-rule check for the engine and library layering.
# Usage: cmake -DRIZ_SOURCE_DIR=<root> -P cmake/check_layering.cmake
#
# Rules (paths relative to <root>):
#   R1  GLFW include outside src/engine/platform/
#   R2  GL include outside src/engine/render.hpp and src/engine/render/
#   R4  forbidden cross-module includes (platform.hpp from render/input/camera/io,
#       render.hpp from platform/input/camera/io)
#   R5  internal header i<Name>.hpp included outside src/<lib>/<name>/,
#       except by white-box tests tests/<name>_*

if(NOT RIZ_SOURCE_DIR)
  message(FATAL_ERROR "riz_layering: pass -DRIZ_SOURCE_DIR=<root>")
endif()

file(GLOB_RECURSE _srcFiles "${RIZ_SOURCE_DIR}/src/*.hpp" "${RIZ_SOURCE_DIR}/src/*.cpp")
file(GLOB _testFiles "${RIZ_SOURCE_DIR}/tests/*.hpp" "${RIZ_SOURCE_DIR}/tests/*.cpp")
set(_files ${_srcFiles} ${_testFiles})

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

  set(_inSrc FALSE)
  if(_rel MATCHES "^src/")
    set(_inSrc TRUE)
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
  if(_rel MATCHES "^src/engine/(render|input|camera|io)[./]" OR _rel STREQUAL "src/engine/topDownCamera.hpp")
    set(_noPlatform TRUE)
  endif()
  # modules that must not see render.hpp
  set(_noRender FALSE)
  if(_rel MATCHES "^src/engine/(platform|input|camera|io)[./]" OR _rel STREQUAL "src/engine/topDownCamera.hpp")
    set(_noRender TRUE)
  endif()

  string(REGEX MATCHALL "(^|\n)[ \t]*#[ \t]*include[ \t]*[<\"][^>\"\n]+[>\"]" _includes "${_content}")
  foreach(_inc IN LISTS _includes)
    string(REGEX REPLACE "^.*#[ \t]*include[ \t]*[<\"]([^>\"\n]+)[>\"]$" "\\1" _path "${_inc}")
    get_filename_component(_name "${_path}" NAME)

    # R1
    if(_inSrc AND _path MATCHES "^GLFW/" AND NOT _inPlatform)
      _violation(R1 "${_rel}" "includes ${_path} outside src/engine/platform/")
    endif()
    # R2
    if(_inSrc AND _path MATCHES "^GL/" AND NOT _rel STREQUAL "src/engine/render.hpp" AND NOT _inRender)
      _violation(R2 "${_rel}" "includes ${_path} outside the render module")
    endif()
    # R4
    if(_inSrc AND _noPlatform AND _name STREQUAL "platform.hpp")
      _violation(R4 "${_rel}" "includes platform.hpp from a module that must not depend on platform")
    endif()
    if(_inSrc AND _noRender AND _name STREQUAL "render.hpp")
      _violation(R4 "${_rel}" "includes ${_name} from a module that must not depend on render")
    endif()

    # R5
    if(_name MATCHES "^i[A-Z][A-Za-z0-9_]*\\.hpp$")
      string(REGEX REPLACE "^i(.*)\\.hpp$" "\\1" _modName "${_name}")
      string(TOLOWER "${_modName}" _modName)
      if(NOT _rel MATCHES "^src/[^/]+/${_modName}/" AND NOT _rel MATCHES "^tests/${_modName}_[^/]*$")
        _violation(R5 "${_rel}" "includes internal header ${_path} outside module ${_modName}")
      endif()
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

set(_RIZ_HEADERS_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/forward_header.hpp.in")

# riz_stage_headers(<outDirVar> <stageName> <header>...)
#   <header> is relative to ${PROJECT_SOURCE_DIR}/src (e.g. engine/render.hpp).
#   Writes forwarders to ${PROJECT_BINARY_DIR}/riz_headers/<stageName>/<header>,
#   removes stale files in that stage dir, sets <outDirVar> to the stage dir in the caller's scope.
function(riz_stage_headers outDirVar stageName)
  set(_headers ${ARGN})
  if(NOT _headers)
    message(FATAL_ERROR "riz_stage_headers(${stageName}): no headers given")
  endif()

  set(_stageDir "${PROJECT_BINARY_DIR}/riz_headers/${stageName}")
  set(_expected "")
  foreach(_header IN LISTS _headers)
    set(_real "${PROJECT_SOURCE_DIR}/src/${_header}")
    if(NOT EXISTS "${_real}")
      message(FATAL_ERROR "riz_stage_headers(${stageName}): header not found: ${_header} (expected ${_real})")
    endif()
    set(RIZ_FORWARD_TARGET "${_real}")
    configure_file("${_RIZ_HEADERS_TEMPLATE}" "${_stageDir}/${_header}" @ONLY)
    list(APPEND _expected "${_stageDir}/${_header}")
  endforeach()

  file(GLOB_RECURSE _present "${_stageDir}/*")
  foreach(_file IN LISTS _present)
    list(FIND _expected "${_file}" _index)
    if(_index EQUAL -1)
      file(REMOVE "${_file}")
    endif()
  endforeach()

  set(${outDirVar} "${_stageDir}" PARENT_SCOPE)
endfunction()

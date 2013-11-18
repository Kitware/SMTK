# find_symbol_in_header_namespace(
#  HEADER_OUT NAMESPACE_OUT
#  "CODE" SNIPPET_IN
#  "HEADERS" HEADERS_IN
#  "NAMESPACES" NAMESPACES_IN
#  "SUMMARY" SUMMARY_IN)
#
# Search for a header file that will allow ${SNIPPET_IN} to compile.
# Each header in the HEADERS_IN list is considered. If found, then
# attempts will be made to compile SNIPPET_IN after including that header.
# The first successful compilation terminates the search.
#
# The code SNIPPET_IN may contain the word NAMESPACE, which will be
# replaced with one of the entries in NAMESPACES_IN using a preprocessor
# macro until the snippet compiles or the list is exhausted.
#
# The snippet may prove compilable in several namespaces, so this macro
# returns two values *and* sets them as CACHE variables:
#   HEADER_OUT    - The name of the header file which allows the snippet to compile.
#   NAMESPACE_OUT - The namespace (including "::") which allows the snippet to compile.
#
# Note that if ${HEADER_OUT} is already a non-empty string, the test is not run.
# Upon return, if a combination of header and namespace were identified,
# then cache variables for HEADER_OUT and NAMESPACE_OUT will be set.
#
# The SUMMARY_IN value is used for informational messages and in the
# descriptions of the cache variables.
#
# Example:
#    find_symbol_in_header_namespace( HASH_FUN_H HASH_FUN_NAMESPACE
#      CODE
#        "int h = NAMESPACE hash<int>()(42);"
#      HEADERS
#        "functional"
#        "tr1/functional"
#        "ext/hash_fun.h"
#        "ext/stl_hash_fun.h"
#        "hash_fun.h"
#        "stl_hash_fun.h"
#        "stl/_hash_fun.h"
#      NAMESPACES
#        "::"
#        "std::"
#        "stdext::"
#        "__gnu_cxx::"
#      SUMMARY "hash")
# This example will set CACHE variables HASH_FUN_H and HASH_FUN_NAMESPACE
# to "ext/hash_fun.h" and "__gnu_cxx::" (respectively) on OS X 10.8;
# to "functional" and "std::" (respectively) on OS X 10.9;
# and so on for each platform where "hash" is provided by one of the HEADERS
# in one of the listed NAMESPACES.

function(find_symbol_in_header_namespace HEADER_OUT NAMESPACE_OUT)

  # Require names for output variables
  if ("${HEADER_OUT}" STREQUAL "" OR "${NAMESPACE_OUT}" STREQUAL "")
    message(FATAL_ERROR "find_symbol_in_header_namespace not passed required HEADER_OUR or NAMESPACE_OUT")
    return()
  endif()

  include(CMakeParseArguments)
  cmake_parse_arguments(_FSIH "" "CODE;SUMMARY" "HEADERS;NAMESPACES" ${ARGN})

  if ("${_FSIH_HEADERS}" STREQUAL "" OR "${_FSIH_NAMESPACES}" STREQUAL "" OR "${_FSIH_CODE}" STREQUAL "")
    message(FATAL_ERROR "find_symbol_in_header_namespace not passed required arguments")
    return()
  endif()

  # Now, only try to find the symbol if the header is *not* defined
  # in the cache. Done in order to save time, which could be expensive
  # as there could be many TRY_COMPILE calls.
  if ("${${HEADER_OUT}}" STREQUAL "")
    foreach(_inc ${_FSIH_HEADERS})
      check_include_file_cxx("${_inc}" _have_header)
      if (_have_header)
        foreach(_ns ${_FSIH_NAMESPACES})
          string(RANDOM LENGTH 10 _tmpf)
          file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/_cmFindSymbol${_tmpf}.cxx
            "#include \"${_inc}\"\n#define NAMESPACE ${_ns}\nint main() {\n  ${_FSIH_CODE};\n  return 0;\n}")
          try_compile(_found_symbol_ns
            ${CMAKE_CURRENT_BINARY_DIR}
            SOURCES ${CMAKE_CURRENT_BINARY_DIR}/_cmFindSymbol${_tmpf}.cxx
            COMPILE_DEFINITIONS "-DHEADER=\"${_inc}\"" "-DNAMESPACE=${_ns}"
            OUTPUT_VARIABLE _result
            )
          file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/_cmFindSymbol${_tmpf}.cxx)
          #message("Tried ${_inc} with ${_ns} result ${_found_symbol_ns}   \"${_result}\"")
          if (_found_symbol_ns)
            set(${HEADER_OUT} "\"${_inc}\"" CACHE STRING "Name of header file containing ${_ns}${_FSIH_SUMMARY}" FORCE)
            set(${NAMESPACE_OUT} ${_ns} CACHE STRING "Namespace containing symbol ${_FSIH_SUMMARY}" FORCE)
            message("    Header <${_inc}> contains ${_ns}${_FSIH_SUMMARY}")
            break()
          endif()
        endforeach()
        if (_found_symbol_ns)
          break()
        endif()
      endif()
    endforeach()
    if (NOT _found_symbol_ns)
      set(${HEADER_OUT} "")
      set(${NAMESPACE_OUT} "")
    endif()
  endif()
endfunction()

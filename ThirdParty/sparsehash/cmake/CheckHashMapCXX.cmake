# CheckHashMapCXX.cmake
#
# Search for a header file that contains the hash<X> template.
# The template may reside in several namespaces, so this macro
# returns two values:
#   HASH_MAP_H           - The name of the header file containing the hash<X> template.
#   HASH_MAP_NAMESPACE   - The namespace (not including "::") in which the template resides.
#
macro(check_hash_map HASH_MAP_H HASH_MAP_NAMESPACE)

  include(FindSymbolInHeaderNamespace)

  # Prefer unordered_map<X,Y>
  find_symbol_in_header_namespace(
    ${HASH_MAP_H}
    ${HASH_MAP_NAMESPACE}
    CODE
      "NAMESPACE ::unordered_map<int,int> foo;"
    HEADERS
      "unordered_map"
      "tr1/unordered_map"
    NAMESPACES
      "std"
      "std::tr1"
    SUMMARY
      "unordered_map"
  )
  # If we don't have unordered_map, see about hash_map
  if ("${${HASH_MAP_H}}" STREQUAL "")
    find_symbol_in_header_namespace(
      ${HASH_MAP_H}
      ${HASH_MAP_NAMESPACE}
      CODE
        "NAMESPACE ::hash_map<int,int> foo;"
      HEADERS
        "hash_map"
        "ext/hash_map"
      NAMESPACES
        "__gnu_cxx"
        ""
        "std"
        "stdext"
      SUMMARY
        "hash_map"
    )
  endif()

endmacro()

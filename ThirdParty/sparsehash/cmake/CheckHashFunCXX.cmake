# CheckHashFunCXX.cmake
#
# Search for a header file that contains the hash<X> template.
# The template may reside in several namespaces, so this macro
# returns two values:
#   HASH_FUN_H           - The name of the header file containing the hash<X> template.
#   HASH_FUN_NAMESPACE   - The namespace (including "::") in which the template resides.
#
macro(check_hash_fun HASH_FUN_H HASH_FUN_NAMESPACE)

  include(FindSymbolInHeaderNamespace)
  find_symbol_in_header_namespace(
    ${HASH_FUN_H}
    ${HASH_FUN_NAMESPACE}
    CODE
      "int h = NAMESPACE hash<int>()(42);"
    HEADERS
      "functional"
      "tr1/functional"
      "ext/hash_fun.h"
      "ext/stl_hash_fun.h"
      "hash_fun.h"
      "stl_hash_fun.h"
      "stl/_hash_fun.h"
    NAMESPACES
      "::"
      "std::"
      "stdext::"
      "__gnu_cxx::"
    SUMMARY
      "hash"
  )

endmacro()

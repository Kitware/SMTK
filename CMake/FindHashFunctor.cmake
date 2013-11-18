# This sets the 5 variable names passed to
# values which identify where hash<X> lives
# on your system and how to specialize it
# to compute hashes of your own class instances.
#
# HASH_FUN_H - the header file containing hash<X>
# HASH_FUN_NAMESPACE - the namespace containing hash<X>
# BEGIN_HASH_NS - C++ code to open the namespace
# END_HASH_NS - C++ code to close the namespace
# HASH_SPECIALIZATION - "1" or "2" depending on how
#    hash should be specialized (see below).
#
# When HASH_SPECIALIZATION is 1 (STRUCT), you should specialize like so:
#     #include ${HASH_FUN_H}
#     class foo {
#     };
#
#     ${BEGIN_HASH_NS}
#       template<>
#       struct hash<foo> {
#         size_t operator() (const foo& f)
#         { return /*something*/; }
#       };
#     ${END_HASH_NS}
#
# When HASH_SPECIALIZATION is 2 (CAST), you should specialize like so:
#    #include ${HASH_FUN_H}
#    class foo {
#    };
#
#    ${BEGIN_HASH_NS}
#      template<>
#      inline size_t hash<foo>::operator() (foo f)
#        { return 0; }
#
#      template<>
#      inline size_t hash<const foo&>::operator() (const foo& f)
#        { return 0; }
#    ${END_HASH_NS}
#
# Note that in case 2 (CAST), you must provide both <foo> and
# <const foo&> specializations.
function(find_hash_functor HASH_FUN_H HASH_FUN_NAMESPACE BEGIN_HASH_NS END_HASH_NS HASH_SPECIALIZATION)

  if (DEFINED ${HASH_SPECIALIZATION})
    return()
  endif()

  include(BeginEndNamespace)
  include(FindSymbolInHeaderNamespace)
  find_symbol_in_header_namespace(
    ${HASH_FUN_H}
    ${HASH_FUN_NAMESPACE}
    CODE
      "int h = NAMESPACE ::hash<int>()(42);"
    HEADERS
      "functional"
      "tr1/functional"
      "ext/hash_fun.h"
      "ext/stl_hash_fun.h"
      "hash_fun.h"
      "stl_hash_fun.h"
      "stl/_hash_fun.h"
    NAMESPACES
      ""
      "std"
      "std::tr1"
      "stdext"
      "__gnu_cxx"
    SUMMARY
      "hash"
  )
  begin_end_namespace("${${HASH_FUN_NAMESPACE}}" ${BEGIN_HASH_NS} ${END_HASH_NS})
  string(RANDOM LENGTH 10 _tmpf)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/_cmHashFun${_tmpf}.cxx"
    "#include ${${HASH_FUN_H}}
class foo {
};

${${BEGIN_HASH_NS}}
  template<>
  struct hash<foo> {
    size_t operator() (const foo& f)
    { return 0; }
  };
${${END_HASH_NS}}

int main()
{
  foo f;
  (void) ${${HASH_FUN_NAMESPACE}}::hash<foo>()(f);
  return 0;
}")
  try_compile(HASH_SPECIALIZATION_IS_STRUCT
    ${CMAKE_CURRENT_BINARY_DIR}
    SOURCES ${CMAKE_CURRENT_BINARY_DIR}/_cmHashFun${_tmpf}.cxx
    OUTPUT_VARIABLE _result
    )
  #message("Tried ${CMAKE_CURRENT_BINARY_DIR}/_cmHashFun${_tmpf}.cxx got ${HASH_SPECIALIZATION_IS_STRUCT} ${_result}")
  if (HASH_SPECIALIZATION_IS_STRUCT)
    set(${HASH_SPECIALIZATION} "1")
  else() # (NOT HASH_SPECIALIZATION_IS_STRUCT)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/_cmHashFun${_tmpf}.cxx"
        "#include ${${HASH_FUN_H}}
class foo {
};

${${BEGIN_HASH_NS}}
  template<>
  inline size_t hash<foo>::operator() (foo f) const
    { return 0; }

  template<>
  inline size_t hash<const foo&>::operator() (const foo& f) const
    { return 0; }
${${END_HASH_NS}}

int main()
{
  foo f;
  (void) ${${HASH_FUN_NAMESPACE}}::hash<foo>()(f);
  return 0;
}")
    try_compile(HASH_SPECIALIZATION_IS_CAST
      ${CMAKE_CURRENT_BINARY_DIR}
      SOURCES ${CMAKE_CURRENT_BINARY_DIR}/_cmHashFun${_tmpf}.cxx
      OUTPUT_VARIABLE _result
    )
    if (HASH_SPECIALIZATION_IS_CAST)
      set(${HASH_SPECIALIZATION} "2")
    endif()
  endif()

  set(${HASH_FUN_H} "${${HASH_FUN_H}}" PARENT_SCOPE)
  set(${HASH_FUN_NAMESPACE} "${${HASH_FUN_NAMESPACE}}" PARENT_SCOPE)
  set(${BEGIN_HASH_NS} "${${BEGIN_HASH_NS}}" PARENT_SCOPE)
  set(${END_HASH_NS} "${${END_HASH_NS}}" PARENT_SCOPE)
  set(${HASH_SPECIALIZATION} "${${HASH_SPECIALIZATION}}" PARENT_SCOPE)
  mark_as_advanced(${HASH_FUN_H} ${HASH_FUN_NAMESPACE} ${BEGIN_HASH_NS} ${END_HASH_NS} ${HASH_SPECIALIZATION})

endfunction()

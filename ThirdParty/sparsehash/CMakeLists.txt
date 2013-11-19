project(sparsehash)

cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH
  "${CMAKE_MODULE_PATH}"
  "${sparsehash_SOURCE_DIR}/cmake"
  )
include(CheckCXXSymbolExists)
include(CheckIncludeFileCXX)
include(CheckTypeSize)
include(CheckHashFunCXX)
include(CheckHashMapCXX)

# Define a configure-time option for changing the namespace
# where the sparsehash classes are located.
set(SPARSEHASH_NAMESPACE "google" CACHE STRING "Choose a namespace you want sparsehash classes to inhabit.")

# Find size-specific type definition header files:
check_include_file_cxx("sys/types.h" SPARSEHASH_HAVE_SYS_TYPES_H)
check_include_file_cxx("stdint.h" SPARSEHASH_HAVE_STDINT_H)
check_include_file_cxx("inttypes" SPARSEHASH_HAVE_INTTYPES_H)
check_include_file_cxx("memcpy.h" SPARSEHASH_HAVE_MEMCPY_H)
check_include_file_cxx("string.h" SPARSEHASH_HAVE_STRING_H)

if (SPARSEHASH_HAVE_SYS_TYPES_H)
  set(_sparsehash_INCLUDES "${_sparsehash_INCLUDES};sys/types.h")
endif()
if (SPARSEHASH_HAVE_STDINT_H)
  set(_sparsehash_INCLUDES "${_sparsehash_INCLUDES};stdint.h")
endif()
if (SPARSEHASH_HAVE_INTTYPES_H)
  set(_sparsehash_INCLUDES "${_sparsehash_INCLUDES};inttypes")
endif()
if (SPARSEHASH_HAVE_MEMCPY_H)
  set(_sparsehash_INCLUDES "${_sparsehash_INCLUDES};memcpy.h")
endif()
if (SPARSEHASH_HAVE_STRING_H)
  set(_sparsehash_INCLUDES "${_sparsehash_INCLUDES};string.h")
endif()

# Now see if we have some common integer types:
set(CMAKE_EXTRA_INCLUDE_FILES ${_sparsehash_INCLUDES})
check_type_size("uint16_t" SIZEOF_UINT16_T)
check_type_size("u_int16_t" SIZEOF_U_INT16_T)
check_type_size("__uint16_t" SIZEOF___UINT16)
check_type_size("long long" SIZEOF_LONG_LONG)
check_cxx_symbol_exists("memcpy" "${_sparsehash_INCLUDES}" SPARSEHASH_HAVE_MEMCPY)

if (SIZEOF_UINT16_T)
  set(SPARSEHASH_HAVE_UINT16_T 1)
endif()
if (SIZEOF_U_INT16_T)
  set(SPARSEHASH_HAVE_U_INT16_T 1)
endif()
if (SIZEOF___UINT16)
  set(SPARSEHASH_HAVE___UINT16 1)
endif()
if (SIZEOF_LONG_LONG)
  set(SPARSEHASH_HAVE_LONG_LONG 1)
endif()

# Locate headers with hash functions, hash sets, and hash maps:
check_hash_fun(HASH_FUN_H HASH_FUN_NAMESPACE)
check_hash_map(HASH_MAP_H HASH_MAP_NAMESPACE)

set(HASH_SET_H ${HASH_MAP_H})
set(HASH_SET_NAMESPACE ${HASH_MAP_NAMESPACE})

set(START_SPARSEHASH_NAMESPACE "namespace ${SPARSEHASH_NAMESPACE} {")
set(END_SPARSEHASH_NAMESPACE "} // namespace ${SPARSEHASH_NAMESPACE}")

mark_as_advanced(
  SPARSEHASH_NAMESPACE
  HASH_FUN_H
  HASH_FUN_NAMESPACE
  HASH_MAP_H
  HASH_MAP_NAMESPACE
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/internal/sparseconfig.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/internal/sparseconfig.h
)

install(
  FILES
    AUTHORS
    COPYING
    doc/dense_hash_map.html
    doc/dense_hash_set.html
    doc/designstyle.css
    doc/implementation.html
    doc/index.html
    doc/performance.html
    doc/sparse_hash_map.html
    doc/sparsetable.html
    doc/sparse_hash_set.html
  DESTINATION
    share/doc/sparsehash
)

install(
  FILES
    dense_hash_map
    dense_hash_set
    sparse_hash_map
    sparse_hash_set
    sparsetable
    template_util.h
    type_traits.h
  DESTINATION
    include/sparsehash
)

install(
  FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/densehashtable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/libc_allocator_with_realloc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/sparsehashtable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/hashtable-common.h
    ${CMAKE_CURRENT_BINARY_DIR}/internal/sparseconfig.h
  DESTINATION
    include/sparsehash/internal
)

#ifndef __sparsehash_sparseconfig_h
#define __sparsehash_sparseconfig_h

/* Describe where and how the hash<X> template is declared.
 */
#define HASH_FUN_H @HASH_FUN_H@
#define HASH_NAMESPACE @HASH_FUN_NAMESPACE@
#define SPARSEHASH_HASH HASH_NAMESPACE ::hash

/* Define the namespace where sparsehash will live */
#define GOOGLE_NAMESPACE @SPARSEHASH_NAMESPACE@
#define _START_GOOGLE_NAMESPACE_ @START_SPARSEHASH_NAMESPACE@
#define _END_GOOGLE_NAMESPACE_ @END_SPARSEHASH_NAMESPACE@

/* Types and headers the platform provides */
#cmakedefine SPARSEHASH_HAVE_UINT16_T
#cmakedefine SPARSEHASH_HAVE_U_INT16_T
#cmakedefine SPARSEHASH_HAVE___UINT16
#cmakedefine SPARSEHASH_HAVE_LONG_LONG
#cmakedefine SPARSEHASH_HAVE_SYS_TYPES_H
#cmakedefine SPARSEHASH_HAVE_STDINT_H
#cmakedefine SPARSEHASH_HAVE_INTTYPES_H
#cmakedefine SPARSEHASH_HAVE_MEMCPY

/* The definitions below are the ones sparsehash actually uses.
 * But cmakedefine would not set them to be exactly "1" when true
 * and this was generating warnings when other config files defined
 * them differently (I'm lookin' at *you* pyconfig.h).
 * It would be nice for sparsehash to prefix its macros properly
 * and get rid of the definitions below.
 */
#ifdef SPARSEHASH_HAVE_UINT16_T
#  define HAVE_UINT16_T 1
#endif
#ifdef SPARSEHASH_HAVE_U_INT16_T
#  define HAVE_U_INT16_T 1
#endif
#ifdef SPARSEHASH_HAVE___UINT16
#  define HAVE___UINT16 1
#endif
#ifdef SPARSEHASH_HAVE_LONG_LONG
#  define HAVE_LONG_LONG 1
#endif
#ifdef SPARSEHASH_HAVE_SYS_TYPES_H
#  define HAVE_SYS_TYPES_H 1
#endif
#ifdef SPARSEHASH_HAVE_STDINT_H
#  define HAVE_STDINT_H 1
#endif
#ifdef SPARSEHASH_HAVE_INTTYPES_H
#  define HAVE_INTTYPES_H 1
#endif
#ifdef SPARSEHASH_HAVE_MEMCPY
#  define HAVE_MEMCPY 1
#endif

#endif // __sparsehash_sparseconfig_h

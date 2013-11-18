#ifndef __sparsehash_sparseconfig_h
#define __sparsehash_sparseconfig_h

#define HASH_FUN_H @HASH_FUN_H@
#define HASH_NAMESPACE @HASH_FUN_NAMESPACE@
#define SPARSEHASH_HASH HASH_NAMESPACE hash

#cmakedefine HAVE_UINT16_T
#cmakedefine HAVE_U_INT16_T
#cmakedefine HAVE___UINT16
#cmakedefine HAVE_LONG_LONG
#cmakedefine HAVE_SYS_TYPES_H
#cmakedefine HAVE_STDINT_H
#cmakedefine HAVE_INTTYPES_H
#cmakedefine HAVE_MEMCPY

#define GOOGLE_NAMESPACE @SPARSEHASH_NAMESPACE@

#define _START_GOOGLE_NAMESPACE_ @START_SPARSEHASH_NAMESPACE@
#define _END_GOOGLE_NAMESPACE_ @END_SPARSEHASH_NAMESPACE@

#endif // __sparsehash_sparseconfig_h

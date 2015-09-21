#ifndef IMESH_MOAB_HPP
#define IMESH_MOAB_HPP

#include "imesh_export.h"

#include "iMesh.h"
#include "MBiMesh.hpp"
#include "moab/Forward.hpp"
#include <cstring>
#include <cstdlib>
#include <cstdio>

using namespace moab;

/* map from MB's entity type to TSTT's entity topology */
extern const iMesh_EntityTopology tstt_topology_table[MBMAXTYPE+1];

/* map from MB's entity type to TSTT's entity type */
extern const iBase_EntityType tstt_type_table[MBMAXTYPE+1];

/* map to MB's entity type from TSTT's entity topology */
extern const EntityType mb_topology_table[MBMAXTYPE+1];

/* map from TSTT's tag types to MOAB's */
extern const DataType mb_data_type_table[iBase_TagValueType_MAX+1];

/* map from MOAB's tag types to tstt's */
extern const iBase_TagValueType tstt_data_type_table[MB_MAX_DATA_TYPE+1];

/* map from MOAB's ErrorCode to tstt's */
extern "C" const iBase_ErrorType iBase_ERROR_MAP[MB_FAILURE+1];

#include "MBiMesh.hpp"

static inline bool iMesh_isError(int code)
  { return (iBase_SUCCESS != code); }
static inline bool iMesh_isError(ErrorCode code)
  { return (MB_SUCCESS != code); }

#define PP_CAT_(a,b) a ## b
#define PP_CAT(a,b) PP_CAT_(a,b)

#define RETURN(CODE)                                                   \
  do {                                                                 \
    *err = MBIMESHI->set_last_error((CODE), "");                       \
    return;                                                            \
  } while(false)

#define ERROR(CODE,MSG)                                                \
  do {                                                                 \
    *err = MBIMESHI->set_last_error((CODE), (MSG));                    \
    return;                                                            \
  } while(false)

#define CHKERR(CODE,MSG)                                               \
  do {                                                                 \
    if (iMesh_isError((CODE)))                                         \
      ERROR((CODE),(MSG));                                             \
  } while(false)

#define CHKENUM(VAL,TYPE,ERR)                                          \
  do {                                                                 \
    if ((VAL) < PP_CAT(TYPE, _MIN) || (VAL) > PP_CAT(TYPE, _MAX))      \
      ERROR((ERR), "Invalid enumeration value");                       \
  } while(false)

// Ensure that a tag's data type matches the expected data type (entity handle
// and entity set handle tags are compatible with one another).
#define CHKTAGTYPE(TAG,TYPE)                                           \
  do {                                                                 \
    int type, result;                                                  \
    iMesh_getTagType(instance, (TAG), &type, &result);                 \
    CHKERR(result, "Couldn't get tag data type");                      \
    if ((type == iBase_ENTITY_HANDLE &&                                \
         (TYPE) == iBase_ENTITY_SET_HANDLE) ||                         \
        (type == iBase_ENTITY_SET_HANDLE &&                            \
         (TYPE) == iBase_ENTITY_HANDLE))                               \
      break;                                                           \
    if (type != (TYPE))                                                \
      ERROR(iBase_INVALID_TAG_HANDLE, "Invalid tag data type");        \
  } while(false)

#define CHKNONEMPTY()                                                  \
  do {                                                                 \
    int count, result;                                                 \
    iMesh_getNumOfType(instance, 0, iBase_ALL_TYPES, &count, &result); \
    CHKERR(result, "Couldn't get number of entities");                 \
    if (count == 0)                                                    \
      ERROR(iBase_INVALID_ENTITY_HANDLE,                               \
            "Invalid entity handle: mesh is empty");                   \
  } while(false)

// Check the array size, and allocate the array if necessary.
// Free the array upon leaving scope unless KEEP_ARRAY
// is invoked.
#define ALLOC_CHECK_ARRAY(array, this_size) \
  iMeshArrayManager array ## _manager ( instance, reinterpret_cast<void**>(array), *(array ## _allocated), *(array ## _size), this_size, sizeof(**array), err ); \
  if (iBase_SUCCESS != *err) return

#define ALLOC_CHECK_TAG_ARRAY(array, this_size) \
  iMeshArrayManager array ## _manager ( instance, reinterpret_cast<void**>(array), *(array ## _allocated), *(array ## _size), this_size, 1, err ); \
  if (iBase_SUCCESS != *err) return

#define KEEP_ARRAY(array) \
  array ## _manager .keep_array()

// Check the array size, and allocate the array if necessary.
// Do NOT free the array upon leaving scope.
#define ALLOC_CHECK_ARRAY_NOFAIL(array, this_size) \
  ALLOC_CHECK_ARRAY(array, this_size); KEEP_ARRAY(array)


// Implement RAII pattern for allocated arrays
class IMESH_EXPORT iMeshArrayManager
{
  void** arrayPtr;

public:


  iMeshArrayManager( iMesh_Instance instance,
                     void** array_ptr,
                     int& array_allocated_space,
                     int& array_size,
                     int count,
                     int val_size,
                     int* err ) : arrayPtr(0)
  {
    if (!array_allocated_space || !*array_ptr) {
      *array_ptr = std::malloc(val_size * count);
      array_allocated_space = array_size = count;
      if (!*array_ptr) {
        ERROR(iBase_MEMORY_ALLOCATION_FAILED, "Couldn't allocate array.");
      }
      arrayPtr = array_ptr;
    }
    else {
      array_size = count;
      if (array_allocated_space < count) {
        ERROR(iBase_BAD_ARRAY_SIZE, 
          "Allocated array not large enough to hold returned contents.");
      }
    }
    RETURN(iBase_SUCCESS);
  }
  
  ~iMeshArrayManager() 
  {
    if (arrayPtr) {
      std::free(*arrayPtr);
      *arrayPtr = 0;
    }
  }
  
  void keep_array()
    { arrayPtr = 0; }
};

inline int compare_no_case(const char *str1, const char *str2, size_t n) {
   for (size_t i = 1; i != n && *str1 && toupper(*str1) == toupper(*str2);
        ++i, ++str1, ++str2);
   return toupper(*str2) - toupper(*str1);
}

// Filter out non-MOAB options and remove the "moab:" prefix
inline std::string filter_options(const char *begin, const char *end)
{
  const char *opt_begin = begin;
  const char *opt_end   = begin;

  std::string filtered;
  bool first = true;

  while (opt_end != end) {
    opt_end = std::find(opt_begin, end, ' ');

    if (opt_end-opt_begin >= 5 && compare_no_case(opt_begin, "moab:", 5) == 0) {
      if (!first)
        filtered.push_back(';');
      first = false;
      filtered.append(opt_begin+5, opt_end);
    }

    opt_begin = opt_end+1;
  }
  return filtered;
}

#endif // IMESH_MOAB_HPP

#ifndef ARRAYMANAGER_HPP
#define ARRAYMANAGER_HPP

#include <cstdlib>

// Check the array size, and allocate the array if necessary.
// Free the array upon leaving scope unless KEEP_ARRAY
// is invoked.
#define ALLOC_CHECK_ARRAY(array, this_size) \
  ArrayManager array ## _manager ( reinterpret_cast<void**>(array), *(array ## _allocated), *(array ## _size), this_size, sizeof(**array), err ); \
  if (iBase_SUCCESS != *err) return

#define ALLOC_CHECK_TAG_ARRAY(array, this_size) \
  ArrayManager array ## _manager ( reinterpret_cast<void**>(array), *(array ## _allocated), *(array ## _size), this_size, 1, err ); \
  if (iBase_SUCCESS != *err) return

#define KEEP_ARRAY(array) \
  array ## _manager .keep_array()

// Check the array size, and allocate the array if necessary.
// Do NOT free the array upon leaving scope.
#define ALLOC_CHECK_ARRAY_NOFAIL(array, this_size) \
  ALLOC_CHECK_ARRAY(array, this_size); KEEP_ARRAY(array)


// Implement RAII pattern for allocated arrays, stolen from iMesh
class ArrayManager
{
  void** arrayPtr;

public:


  ArrayManager( void** array_ptr,
                int& array_allocated_space,
                int& array_size,
                int count,
                int val_size,
                int* err ) : arrayPtr(0)
  {
    if (!*array_ptr || !array_allocated_space) {
      *array_ptr = std::malloc(val_size * count);
      array_allocated_space = array_size = count;
      if (!*array_ptr) {
        *err = iBase_MEMORY_ALLOCATION_FAILED;
        return;
      }
      arrayPtr = array_ptr;
    }
    else {
      array_size = count;
      if (array_allocated_space < count) {
        *err = iBase_BAD_ARRAY_DIMENSION;
        return;
      }
    }

    *err = iBase_SUCCESS;
  }

  ~ArrayManager()
  {
    if (arrayPtr) {
      std::free(*arrayPtr);
      *arrayPtr = 0;
    }
  }

  void keep_array()
    { arrayPtr = 0; }
};

#endif

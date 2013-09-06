
#this function will define a parent scope
#variable with the name of variable you pass in
#if you need to find boost it will also define
#a variable named ${type}_BOOST_TRUE

function(determineSharedPtrType type incType)

  set(BOOST_TYPE 3)
  set(RESULT 0)
  set(SHARED_PTR_TYPE_FOUND FALSE)

  if(NOT ${SHARED_PTR_TYPE_FOUND})
    try_compile(SHARED_PTR_TYPE_FOUND
      ${PROJECT_BINARY_DIR}/CMakeTmp
      ${PROJECT_SOURCE_DIR}/CMake/shared_ptr.cxx
      )
    if(${SHARED_PTR_TYPE_FOUND})
      set(RESULT "std")
      set(INCLUDE_RESULT "#include <memory>")
    endif()
  endif()

  if(NOT ${SHARED_PTR_TYPE_FOUND})
  try_compile(SHARED_PTR_TYPE_FOUND
    ${PROJECT_BINARY_DIR}/CMakeTmp
    ${PROJECT_SOURCE_DIR}/CMake/shared_ptr_tr1.cxx
    )
    if(${SHARED_PTR_TYPE_FOUND})
      set(RESULT "std::tr1")
      set(INCLUDE_RESULT "#include <tr1/memory>")
    endif()
  endif()

  if(NOT ${SHARED_PTR_TYPE_FOUND})
    set(RESULT "boost")
    set(INCLUDE_RESULT "#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>")
    set(${type}_BOOST_TRUE TRUE PARENT_SCOPE)
  endif()


  set(${type} ${RESULT} PARENT_SCOPE)
  set(${incType} ${INCLUDE_RESULT} PARENT_SCOPE)

endfunction(determineSharedPtrType)


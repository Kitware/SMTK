#this function will define a parent scope
#variable with the name of variable you pass in
#if you need to find boost it will also define
#a variable named ${type}_BOOST_TRUE

function(determineSharedPtrType type incType)

  set(RESULT)
  set(SHARED_PTR_TYPE_FOUND FALSE)

  #for support of owner_less we mark
  #boost as supporting it, and also
  #we mark c++11
  set(HAS_OWNER_LESS 0 PARENT_SCOPE)

  if(NOT ${SHARED_PTR_TYPE_FOUND})
    try_compile(SHARED_PTR_TYPE_FOUND
      ${PROJECT_BINARY_DIR}/CMakeTmp
      ${PROJECT_SOURCE_DIR}/CMake/shared_ptr.cxx
      )
    if(${SHARED_PTR_TYPE_FOUND})
      set(RESULT "std")
      set(INCLUDE_RESULT "#include <memory>")
      set(HAS_OWNER_LESS 1 PARENT_SCOPE)
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
    set(INCLUDE_RESULT "#ifndef SHIBOKEN_SKIP
#  include <boost/shared_ptr.hpp>
#  include <boost/enable_shared_from_this.hpp>
#  include <boost/make_shared.hpp>
#  include <boost/weak_ptr.hpp>
#  include <boost/smart_ptr/owner_less.hpp>
#endif")
    set(${type}_BOOST_TRUE TRUE PARENT_SCOPE)
    set(HAS_OWNER_LESS 1 PARENT_SCOPE)
  endif()


  set(${type} ${RESULT} PARENT_SCOPE)
  set(${incType} ${INCLUDE_RESULT} PARENT_SCOPE)

endfunction(determineSharedPtrType)

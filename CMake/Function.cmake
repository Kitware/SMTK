# This function will determine which namespace
# to pull the C++-11 "function" and "bind"
# functionality from.
function(determineFunctionType type incType)

  set(RESULT)
  set(FUNCTION_TYPE_FOUND FALSE)

  if(NOT ${FUNCTION_TYPE_FOUND})
    try_compile(FUNCTION_TYPE_FOUND
      ${PROJECT_BINARY_DIR}/CMakeTmp
      ${PROJECT_SOURCE_DIR}/CMake/function.cxx
    )
    if(${FUNCTION_TYPE_FOUND})
      set(RESULT "std")
      set(INCLUDE_RESULT "#include <functional>")
    endif()
  endif()

  if(NOT ${FUNCTION_TYPE_FOUND})
    try_compile(FUNCTION_TYPE_FOUND
      ${PROJECT_BINARY_DIR}/CMakeTmp
      ${PROJECT_SOURCE_DIR}/CMake/function_tr1.cxx
    )
    if(${FUNCTION_TYPE_FOUND})
      set(RESULT "std::tr1")
      set(INCLUDE_RESULT "#include <tr1/functional>")
    endif()
  endif()

  if(NOT ${FUNCTION_TYPE_FOUND})
    set(RESULT "boost")
    set(INCLUDE_RESULT "#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/functional.hpp>
#include <boost/mpl/placeholders.hpp> // for _1, _2, ..., _N
")
    set(${type}_BOOST_TRUE TRUE PARENT_SCOPE)
  endif()


  set(${type} ${RESULT} PARENT_SCOPE)
  set(${incType} ${INCLUDE_RESULT} PARENT_SCOPE)

endfunction(determineFunctionType)

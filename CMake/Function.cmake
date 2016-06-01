# This function will determine which namespace
# to pull the C++-11 "function", "bind", and
# "placeholders" functionality from.
function(determineFunctionType found type ptype incType)

  set(RESULT)
  set(PLACEHOLDERS_RESULT)
  set(FUNCTION_TYPE_FOUND FALSE)

  if(NOT ${FUNCTION_TYPE_FOUND})
    try_compile(FUNCTION_TYPE_FOUND
      ${PROJECT_BINARY_DIR}/CMakeTmp
      ${PROJECT_SOURCE_DIR}/CMake/function.cxx
    )
    if(${FUNCTION_TYPE_FOUND})
      set(RESULT "std")
      set(PLACEHOLDERS_RESULT "
        using ${RESULT}::placeholders::_1;
        using ${RESULT}::placeholders::_2;
        using ${RESULT}::placeholders::_3;
        using ${RESULT}::placeholders::_4;
      ")
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
      set(PLACEHOLDERS_RESULT "${RESULT}")
      set(PLACEHOLDERS_RESULT "
        using ${RESULT}::placeholders::_1;
        using ${RESULT}::placeholders::_2;
        using ${RESULT}::placeholders::_3;
        using ${RESULT}::placeholders::_4;
      ")
      set(INCLUDE_RESULT "#include <tr1/functional>")
    endif()
  endif()

  if(NOT ${FUNCTION_TYPE_FOUND})
    try_compile(FUNCTION_TYPE_FOUND
      ${PROJECT_BINARY_DIR}/CMakeTmp
      ${PROJECT_SOURCE_DIR}/CMake/function_boost.cxx
      CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${Boost_INCLUDE_DIRS}"
    )
    if(${FUNCTION_TYPE_FOUND})
      set(RESULT "boost")
      set(PLACEHOLDERS_RESULT "
        // This is empty when using Boost as it puts the
        // placeholders in an anonymous namespace. Just put
        //     \"using namespace smtk::placeholders;\"
        // in your code anyway (for people using C++11 or tr1).
      ")
      set(INCLUDE_RESULT "#ifndef SHIBOKEN_SKIP
#  include <boost/function.hpp>
#  include <boost/bind.hpp>
#  include <boost/functional.hpp>
#  include <boost/mpl/placeholders.hpp> // for _1, _2, ..., _N
#endif
")
      set(${type}_BOOST_TRUE TRUE PARENT_SCOPE)
    endif()
  endif()

  if (FUNCTION_TYPE_FOUND)
    set(${type} ${RESULT} PARENT_SCOPE)
    set(${ptype} ${PLACEHOLDERS_RESULT} PARENT_SCOPE)
    set(${incType} ${INCLUDE_RESULT} PARENT_SCOPE)
    set(${found} TRUE PARENT_SCOPE)
  else()
    set(${found} PARENT_SCOPE) # unsets ${found}
  endif()

endfunction(determineFunctionType)

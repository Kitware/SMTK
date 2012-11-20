cmake_minimum_required (VERSION 2.8)
project(Slctk)

# include some cmake code that builds automatic header tests
# and can targets for unit tests
include(CMake/SlctkMacros.cmake)

# include export header so that we can easily expose symbols
# in dynamic libraries
include(GenerateExportHeader)

# Include mechanism for determining shared and weak pointer support
include(CMake/SharedPtr.cmake)

determineSharedPtrType(POINTER_NAMESPACE POINTER_TYPE_HEADERS)

#if the shared_ptr isn't able to be
#found in c++ or c++ tr1 we include boost
if(${POINTER_NAMESPACE_BOOST_TRUE})
  find_package(Boost)
  include_directories(
  ${Boost_INCLUDE_DIR}
  )
endif()

configure_file(
  ${PROJECT_SOURCE_DIR}/CMake/PublicPointerDefs.h.in
  ${PROJECT_BINARY_DIR}/PublicPointerDefs.h
  @ONLY)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${Slctk_SOURCE_DIR}/CMake)

option(Slctk_ENABLE_TESTING "Enable Testing" ON)
#option(Slctk_ENABLE_EXAMPLES "Enable Examples" ON)

#turn on ctest if we want testing
if (Slctk_ENABLE_TESTING)
  enable_testing()
  include(CTest)
endif()


## Set the directory where the binaries will be stored
set( EXECUTABLE_OUTPUT_PATH
  ${PROJECT_BINARY_DIR}/bin
  CACHE PATH
  "Directory where all executable will be stored"
)

## Set the directory where the libraries will be stored
set( LIBRARY_OUTPUT_PATH
  ${PROJECT_BINARY_DIR}/libs
  CACHE PATH
  "Directory where all the libraries will be stored"
)
mark_as_advanced(
  EXECUTABLE_OUTPUT_PATH
  LIBRARY_OUTPUT_PATH)

#add this directory so we can include files like: <slctk/internal/Test.h>
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  )


#add the actual code
add_subdirectory(attribute)
add_subdirectory(model)

#add the examples code
if(Slctk_ENABLE_EXAMPLES)
  add_subdirectory(Examples)
endif()

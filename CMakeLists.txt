cmake_minimum_required (VERSION 2.8.10)
project(SMTK)

#Add our Cmake directory to the module search path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${SMTK_SOURCE_DIR}/CMake)

# include export header so that we can easily expose symbols
# in dynamic libraries
include(GenerateExportHeader)

# include cmake code that builds automatic header tests,
# makes targets for unit tests, and makes it easier to generate
# export install targets
include(CMake/SMTKMacros.cmake)

# Include mechanism for determining shared and weak pointer support
include(CMake/SharedPtr.cmake)


################################################################################
# Options that the user controls
################################################################################
option(BUILD_SHARED_LIBS "Build SMTK using shared libraries" OFF)
# Option to build Qt ui compoments for attributes
option(SMTK_BUILD_QT "Build Qt GUI" OFF)
option(SMTK_ENABLE_TESTING "Enable Testing" ON)
option(SMTK_BUILD_PYTHON_WRAPPINGS "Build Python Wrappings using Shiboken" OFF)

################################################################################
# Testing Related Settings
################################################################################

#turn on ctest if we want testing
if (SMTK_ENABLE_TESTING)
  enable_testing()
  include(CTest)
endif()

################################################################################
# Boost Related Settings
################################################################################

find_package(Boost 1.50.0 REQUIRED)
include_directories("${Boost_INCLUDE_DIRS}")

################################################################################
# Qt Related Settings
################################################################################

#make find qt4 bring in import targets which are far nicer to use
if(SMTK_BUILD_QT)
  set(QT_USE_IMPORTED_TARGETS TRUE)
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
endif()


################################################################################
# Wrapping Related Settings
################################################################################
if(SMTK_BUILD_PYTHON_WRAPPINGS)
  include(UseShiboken)
endif()

################################################################################
# Determine shared_ptr
################################################################################
determineSharedPtrType(POINTER_NAMESPACE POINTER_TYPE_HEADERS)

#if the shared_ptr isn't able to be
#found in c++ or c++ tr1 we include boost
if(${POINTER_NAMESPACE_BOOST_TRUE})
  find_package(Boost)
  include_directories( ${Boost_INCLUDE_DIR} )
endif()

configure_file(
  ${PROJECT_SOURCE_DIR}/CMake/SharedPtr.h.in
  ${PROJECT_BINARY_DIR}/smtk/SharedPtr.h
  @ONLY)

install (FILES ${PROJECT_BINARY_DIR}/smtk/SharedPtr.h
  DESTINATION include/smtk)


################################################################################
# Install Related Settings
################################################################################

## Set the directory where the binaries will be stored
set( EXECUTABLE_OUTPUT_PATH
  ${PROJECT_BINARY_DIR}/bin
  CACHE PATH
  "Directory where all executable will be stored"
)

## Set the directory where the libraries will be stored
set( LIBRARY_OUTPUT_PATH
  ${PROJECT_BINARY_DIR}/lib
  CACHE PATH
  "Directory where all the libraries will be stored"
)
mark_as_advanced(
  EXECUTABLE_OUTPUT_PATH
  LIBRARY_OUTPUT_PATH)


################################################################################
# Build third party libraries
################################################################################

add_subdirectory(ThirdParty)

################################################################################
# Include Dirs Settings
################################################################################

#add this directory so we can include files like: <smtk/internal/Test.h>
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/cJSON
  )

################################################################################
# Finally build smtk
################################################################################

#Build the actual code
add_subdirectory(smtk)

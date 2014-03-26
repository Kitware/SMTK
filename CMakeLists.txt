cmake_minimum_required (VERSION 2.8.11)
cmake_policy(SET CMP0020 NEW)
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

# Include mechanism for determining how to specialize hash<X>()
include(CMake/FindHashFunctor.cmake)

################################################################################
# Options that the user controls
################################################################################
option(BUILD_SHARED_LIBS "Build SMTK using shared libraries" OFF)
option(Smtk_NO_SYSTEM_BOOST "Allow boost to search for system installed boost" ON)
# Option to build Qt ui compoments for attributes
option(SMTK_BUILD_QT "Build Qt GUI" OFF)
option(SMTK_BUILD_ParaView "Build ParaView component" OFF)
option(SMTK_BUILD_CGM "Build CGM component" OFF)
option(SMTK_ENABLE_TESTING "Enable Testing" ON)
option(SMTK_BUILD_PYTHON_WRAPPINGS "Build Python Wrappings using Shiboken" OFF)
option(SMTK_USE_SYSTEM_SPARSEHASH "Use the system-installed sparsehash?" OFF)
option(SMTK_HASH_STORAGE "Use sparsehash library for primary storage?" OFF)
set(SMTK_DATA_DIR "" CACHE PATH "Path to a directory of SMTK test data.")
mark_as_advanced(SMTK_USE_SYSTEM_SPARSEHASH SMTK_HASH_STORAGE)

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

# Boost is required for boost::uuids::uuid and, depending
# on the platform, for boost::shared_ptr, as well as for filesystem support.
#setup if we should use boost static libraries based on if we are
#building static or shared. We need to match boosts library type to ours so
#that we handle symbol visibility properly
if(${BUILD_SHARED_LIBS})
  set(Boost_USE_STATIC_LIBS OFF)
else()
  set(Boost_USE_STATIC_LIBS ON)
endif()

#if boost is installed as a system library on a machine, we will never
#be able to set the superbuild to use the custom boost version. The solution
#is that when doing a system
set(Boost_NO_SYSTEM_PATHS ${Smtk_NO_SYSTEM_BOOST})

# List of Boost features used:
# * Filesystem
# * Scoped Ptr
# * Shared Ptr
# * String algorithms
# * UUID Generation
find_package(Boost 1.50.0
             COMPONENTS   filesystem system  REQUIRED)

#setup windows exception handling so we can compile properly with boost enabled
if(WIN32 AND MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif()

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
# ParaView-VTK Related Settings
################################################################################

if(SMTK_BUILD_ParaView)
  find_package(ParaView REQUIRED)
  include("${PARAVIEW_USE_FILE}")
  include(ParaViewPlugins)
  list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")
  set(VTK_MAKE_INSTANTIATORS TRUE CACHE INTERNAL "" FORCE)
endif()


################################################################################
# CGM Related Settings
################################################################################

if(SMTK_BUILD_CGM)
  find_package(CGM REQUIRED)
endif(SMTK_BUILD_CGM)

################################################################################
# Sparsehash Related Settings
################################################################################

if (SMTK_USE_SYSTEM_SPARSEHASH)
  # A system sparsehash implementation must be detected with pkgconfig:
  set(SYSTEM_SPARSEHASH_REQUIRED "REQUIRED")
  include(FindPkgConfig)
  pkg_check_modules(SPARSEHASH ${SYSTEM_SPARSEHASH_REQUIRED} libsparsehash)
else()
  set(SPARSEHASH_INCLUDE_DIRS
    "${SMTK_SOURCE_DIR}/ThirdParty"
    "${SMTK_BINARY_DIR}/ThirdParty")
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

configure_file(
  ${PROJECT_SOURCE_DIR}/CMake/SharedPtr.h.in
  ${PROJECT_BINARY_DIR}/smtk/SharedPtr.h
  @ONLY)

install (FILES ${PROJECT_BINARY_DIR}/smtk/SharedPtr.h
  DESTINATION include/smtk)

################################################################################
# Determine hash<X> specialization
################################################################################
find_hash_functor(SMTK_HASH_H SMTK_HASH_NS SMTK_HASH_BEGIN_NS SMTK_HASH_END_NS SMTK_HASH_SPECIALIZATION)
configure_file(
  ${PROJECT_SOURCE_DIR}/CMake/HashFunctor.h.in
  ${PROJECT_BINARY_DIR}/smtk/HashFunctor.h
  @ONLY)

install (FILES ${PROJECT_BINARY_DIR}/smtk/HashFunctor.h
  DESTINATION include/smtk)


################################################################################
# Save compile-time options for use by other packages
################################################################################
configure_file(
  ${PROJECT_SOURCE_DIR}/CMake/options.h.in
  ${PROJECT_BINARY_DIR}/smtk/options.h
  @ONLY)

install (FILES ${PROJECT_BINARY_DIR}/smtk/options.h
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
# Build documentation
################################################################################

add_subdirectory(doc)

################################################################################
# Include Dirs Settings
################################################################################

# Add the current directory so we can include files like: <smtk/internal/Test.h>
# Save the include directories in a variable so that VTK and ParaView modules
# (inside the paraview/{vtk,pvvtk} directories) can reuse them.
include_directories(
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${SPARSEHASH_INCLUDE_DIRS}"
  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/cJSON"
  "${Boost_INCLUDE_DIRS}"
)

################################################################################
# Look for VTK and ParaView modules
################################################################################
# Cannot prepare ParaView/VTK modules until the include directories
# have been set or they will be unable to find SMTK, Boost, etc.
if(SMTK_BUILD_ParaView)
  pv_setup_module_environment("SMTK")
  pv_process_modules()
endif()

################################################################################
# Finally build smtk
################################################################################

#Build the actual code
add_subdirectory(smtk)

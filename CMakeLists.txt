cmake_minimum_required (VERSION 2.8.11)
cmake_policy(SET CMP0020 NEW)
if (POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif()
if (POLICY CMP0054)
  cmake_policy(SET CMP0054 OLD)
endif()
project(SMTK)

################################################################################
# SMTK version number

set(SMTK_VERSION_MAJOR 0)
set(SMTK_VERSION_MINOR 9)
set(SMTK_VERSION_PATCH 0)
set(SMTK_VERSION
  "${SMTK_VERSION_MAJOR}.${SMTK_VERSION_MINOR}.${SMTK_VERSION_PATCH}")
#
################################################################################

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

# Include mechanism for determining function and bind support
include(CMake/Function.cmake)

# Include mechanism for determining how to specialize hash<X>()
include(CMake/FindHashFunctor.cmake)

################################################################################
# Options that the user controls
################################################################################
option(BUILD_SHARED_LIBS "Build SMTK using shared libraries" OFF)
option(SMTK_NO_SYSTEM_BOOST "Allow boost to search for system installed boost" ON)
# Option to build Qt ui compoments for attributes
option(SMTK_BUILD_QT "Build Qt GUI" OFF)
option(SMTK_BUILD_VTK "Build VTK component" OFF)
option(SMTK_BUILD_CGM "Build CGM component" OFF)
option(SMTK_BUILD_DISCRETE_SESSION "Build discrete model session components" OFF)
option(SMTK_BUILD_SESSION_PLUGIN "Build paraview plugin for model sessions" OFF)

if (SMTK_BUILD_VTK)
  option(SMTK_ENABLE_EXODUS_SESSION "Build a session to Exodus-II side sets" ON)
endif()

option(SMTK_ENABLE_REMUS "Build Remus component" OFF)
option(SMTK_ENABLE_TESTING "Enable Testing" ON)
option(SMTK_BUILD_PYTHON_WRAPPINGS "Build Python Wrappings using Shiboken" OFF)
option(SMTK_USE_SYSTEM_SPARSEHASH "Use the system-installed sparsehash?" OFF)
option(SMTK_HASH_STORAGE "Use sparsehash library for primary storage?" OFF)
set(SMTK_DATA_DIR "" CACHE PATH "Path to a directory of SMTK test data.")
mark_as_advanced(SMTK_USE_SYSTEM_SPARSEHASH SMTK_HASH_STORAGE)

option(SMTK_ENABLE_DOCUMENTATION
  "Include targets for Doxygen- and Sphinx-generated documentation" OFF)
if (SMTK_ENABLE_DOCUMENTATION)
  find_package(Doxygen)
  find_package(Sphinx)
endif()

################################################################################
# Testing Related Settings
################################################################################

#turn on ctest if we want testing
if (SMTK_ENABLE_TESTING)
  message(STATUS "SMTK_ENABLE_TESTING")
  enable_testing()
  include(CTest)

  #add the first test which is for checking the copyright
  add_test(NAME CopyrightStatement
    COMMAND ${CMAKE_COMMAND}
        "-DSMTK_SOURCE_DIR=${SMTK_SOURCE_DIR}"
        -P "${SMTK_SOURCE_DIR}/CMake/CheckCopyright.cmake")
endif()

################################################################################
# Boost Related Settings
################################################################################

# Boost is required for boost::uuids::uuid and, depending
# on the platform, for boost::shared_ptr, as well as for filesystem support.
#setup if we should use boost static libraries based on if we are
#building static or shared. We need to match boosts library type to ours so
#that we handle symbol visibility properly, plus at the same time we have
#to understand that on some platforms or builds we will only have the static
#libraries so we need to handle that when presented
if(NOT DEFINED Boost_USE_STATIC_LIBS)
  if(${BUILD_SHARED_LIBS})
    set(Boost_USE_STATIC_LIBS OFF)
  else()
    set(Boost_USE_STATIC_LIBS ON)
  endif()
endif()

#if boost is installed as a system library on a machine, we will never
#be able to set the superbuild to use the custom boost version. The solution
#is that when doing a system
set(Boost_NO_SYSTEM_PATHS ${SMTK_NO_SYSTEM_BOOST})

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
# NOTE: This has be before any find_package(VTK), because currently ParaView
# and VTK can NOT be easily switched due to modules and macros intermixed
################################################################################
################################################################################
# Look for Discrete session discrete model modules
################################################################################

if(SMTK_BUILD_SESSION_PLUGIN)
  find_package(ParaView REQUIRED)
endif()

################################################################################
# VTK Related Settings
################################################################################

if(SMTK_BUILD_VTK)
  if(SMTK_BUILD_SESSION_PLUGIN)
    find_package(ParaView REQUIRED)
  else()
    # Find the package here so environment variables are set, but
    # be careful not to include the VTK_USE_FILE until inside
    # a subdirectory that requires it; otherwise the list of include
    # paths can become too long for some platforms.
    find_package(VTK REQUIRED)
  endif()
endif()

# Maintain compatibility with older VTK/ParaView, check for the definition
# of, and define if not, VTK_RENDERING_BACKEND.
if(NOT DEFINED VTK_RENDERING_BACKEND)
  set(VTK_RENDERING_BACKEND "OpenGL")
endif()

################################################################################
# CGM Related Settings
################################################################################

if(SMTK_BUILD_CGM)
  find_package(CGM REQUIRED)
endif(SMTK_BUILD_CGM)

################################################################################
# Remus related settings
################################################################################

if (SMTK_ENABLE_REMUS)
  find_package(Remus REQUIRED)
endif()

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
    "${SMTK_SOURCE_DIR}/thirdparty"
    "${SMTK_BINARY_DIR}/thirdparty")
endif()

################################################################################
# Wrapping Related Settings
################################################################################
if(SMTK_BUILD_PYTHON_WRAPPINGS)
  include(UseShiboken)
  find_package(PythonInterp 2.7 REQUIRED)
  # Set up environment variables needed to import the python modules for tests.
  if (PYTHONINTERP_FOUND AND SMTK_ENABLE_TESTING)
    get_filename_component(SHIBOKEN_LIBRARY_PATH ${SHIBOKEN_LIBRARY} PATH)
    set(LIB_ENV_VAR "")
    if(APPLE AND UNIX)
      set(LIB_ENV_VAR "DYLD_FALLBACK_LIBRARY_PATH=${SHIBOKEN_LIBRARY_PATH}:${LIBRARY_OUTPUT_PATH}")
    elseif(UNIX)
      set(LIB_ENV_VAR "LD_LIBRARY_PATH=${SHIBOKEN_LIBRARY_PATH}:${LIBRARY_OUTPUT_PATH}")
    endif()
    set(SHIBOKEN_SMTK_PYTHON "")
    if(UNIX)
      set(SHIBOKEN_SMTK_PYTHON "${CMAKE_BINARY_DIR}:${SHIBOKEN_LIBRARY_PATH}")
    else() #windows
      set(SHIBOKEN_SMTK_PYTHON "${CMAKE_BINARY_DIR};${SHIBOKEN_LIBRARY_PATH}")
    endif()
  endif()
  #message("${LIB_ENV_VAR}   PYTHONPATH=${SIBOKEN_SMTK_PYTHON}")
  #message("${SHIBOKEN_LIBRARY_PATH}")
endif()

################################################################################
# Determine function
################################################################################
determineFunctionType(
  FUNCTION_FOUND
  FUNCTION_NAMESPACE
  USING_PLACEHOLDERS
  FUNCTION_TYPE_HEADERS)

if (NOT FUNCTION_FOUND)
  message(FATAL_ERROR
    "
    SMTK requires std::function, std::tr1::function, or boost::function.
    Please examine CMake's logs to see why CMake/function*.cxx won't compile.
    ")
endif()

configure_file(
  ${PROJECT_SOURCE_DIR}/CMake/Function.h.in
  ${PROJECT_BINARY_DIR}/smtk/Function.h
  @ONLY)

install (FILES ${PROJECT_BINARY_DIR}/smtk/Function.h
  DESTINATION include/smtk)

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
  ${PROJECT_SOURCE_DIR}/CMake/Options.h.in
  ${PROJECT_BINARY_DIR}/smtk/Options.h
  @ONLY)

install (FILES ${PROJECT_BINARY_DIR}/smtk/Options.h
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

# Install rules for SMTK macros usable by external packages:
install(
  FILES
    ${PROJECT_SOURCE_DIR}/CMake/EncodeCStringFunctions.cmake
    ${PROJECT_SOURCE_DIR}/CMake/SMTKOperatorXML.cmake
    ${PROJECT_SOURCE_DIR}/CMake/SMTKSessionJSON.cmake
  DESTINATION
    lib/cmake/SMTK
)

################################################################################
# Build third party libraries
################################################################################

add_subdirectory(thirdparty)

################################################################################
# Include Dirs Settings
################################################################################

# Add the current directory so we can include files like: <smtk/internal/Test.h>
# Save the include directories in a variable so that VTK modules
# (inside the extension/vtk and bridge/discrete directories) can reuse them.
include_directories(
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${SPARSEHASH_INCLUDE_DIRS}"
  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cJSON"
  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml"
  "${Boost_INCLUDE_DIRS}"
)

################################################################################
# Look for Discrete session discrete model modules
################################################################################

if(SMTK_BUILD_DISCRETE_SESSION)
  if(SMTK_BUILD_SESSION_PLUGIN)
    find_package(ParaView REQUIRED)
  else()
    find_package(VTK REQUIRED)
  endif()

  # This option is only available when SMTK_BUILD_DISCRETE_SESSION is ON
  option(SMTK_BUILD_MOAB_READER "Build moab reader for discrete model" OFF)
endif()

################################################################################
# Finally build smtk
################################################################################

add_subdirectory(smtk)

################################################################################
# Build documentation
# This also includes tutorials and other documentation that has its
# source checked against SMTK libraries, so it must come after those
# targets have been declared.
################################################################################

if (SMTK_ENABLE_DOCUMENTATION)
  add_subdirectory(doc)
endif()

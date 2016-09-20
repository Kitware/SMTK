cmake_minimum_required(VERSION 3.5)

cmake_policy(SET CMP0020 NEW)
if (POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif()
if (POLICY CMP0046)
  cmake_policy(SET CMP0046 NEW)
endif()

#If the user/superbuild hasn't explicitly stated what c++ standard to use
#require C++11
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED True)
  set(CMAKE_CXX_EXTENSIONS FALSE)
endif()

project(SMTK)

# Remove SMTKTargets.cmake at the beginning of each CMake run; each time
# a library is declared below, the file is appended and we do not want
# to keep appending over multiple runs.
file(REMOVE ${PROJECT_BINARY_DIR}/SMTKTargets.cmake)

################################################################################
# SMTK version number

set(SMTK_VERSION_MAJOR 1)
set(SMTK_VERSION_MINOR 0)
set(SMTK_VERSION_PATCH 0)
set(SMTK_VERSION
  "${SMTK_VERSION_MAJOR}.${SMTK_VERSION_MINOR}.${SMTK_VERSION_PATCH}")
#
################################################################################

#Add our Cmake directory to the module search path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${SMTK_SOURCE_DIR}/CMake)

# Include GNU install directory module to detect where to install
# files on Linux/Unix systems (e.g., lib vs lib64)
include(GNUInstallDirs)

# Include export header so that we can easily expose symbols
# in dynamic libraries
include(GenerateExportHeader)

# Include cmake code that builds automatic header tests,
# makes targets for unit tests, and makes it easier to generate
# export install targets
include(SMTKMacros)

# include cmake code that allows for easier testing of SMTK classes
# and also provides helpers around create_test_sourcelist
include(CMake/SMTKTestingMacros.cmake)

# Include mechanism for determining function and bind support
include(Function)

# Include mechanism for determining how to specialize hash<X>()
include(FindHashFunctor)

################################################################################
## Paths for output

# Set the directory where the binaries will be stored
set(EXECUTABLE_OUTPUT_PATH
  ${PROJECT_BINARY_DIR}/bin)

# Set the directory where the libraries will be stored
set(LIBRARY_OUTPUT_PATH
  ${PROJECT_BINARY_DIR}/lib)

include(CMakeDependentOption)

################################################################################
# Options that the user controls
################################################################################
option(BUILD_SHARED_LIBS "Build SMTK using shared libraries" ON)
option(SMTK_NO_SYSTEM_BOOST "Allow boost to search for system installed boost" ON)
# Option to build Qt ui compoments for attributes
option(SMTK_ENABLE_TESTING "Enable Testing" ON)
option(SMTK_ENABLE_EXAMPLES "Enable Examples" OFF)
option(SMTK_ENABLE_PYTHON_WRAPPING "Build Python Wrappings using Shiboken" OFF)
# Provide system packagers with the ability to install SMTK
# to the system's Python site package directory. The default
# is off so that people building relocatable bundles (such as
# CMB) can distribute their own Python interpreter with
# internal packages (e.g., as part of a Mac ".app" bundle).
cmake_dependent_option(
  SMTK_INSTALL_PYTHON_TO_SITE_PACKAGES
  "Install Python modules to the interpreter's site-packages directory or into CMAKE_INSTALL_PREFIX?"
  OFF
  SMTK_ENABLE_PYTHON_WRAPPING OFF)
mark_as_advanced(SMTK_INSTALL_PYTHON_TO_SITE_PACKAGES)
option(SMTK_ENABLE_QT_SUPPORT "Build Qt GUI" OFF)
option(SMTK_ENABLE_VTK_SUPPORT "Build VTK component" ON)
option(SMTK_ENABLE_REMUS_SUPPORT "Build Remus components" OFF)
cmake_dependent_option(SMTK_ENABLE_PARAVIEW_SUPPORT "Build paraview plugins for model sessions" OFF
  SMTK_ENABLE_VTK_SUPPORT OFF)

option(SMTK_ENABLE_CGM_SESSION "Build CGM component" OFF)
cmake_dependent_option(SMTK_ENABLE_DISCRETE_SESSION "Build discrete model session components" OFF
  SMTK_ENABLE_VTK_SUPPORT OFF)
cmake_dependent_option(SMTK_ENABLE_MOAB_DISCRETE_READER "Build moab reader for discrete model" OFF
  "SMTK_ENABLE_DISCRETE_SESSION" OFF)
cmake_dependent_option(SMTK_ENABLE_POLYGON_SESSION "Build Boost.polygon model session." ON
  SMTK_ENABLE_VTK_SUPPORT OFF)
cmake_dependent_option(SMTK_ENABLE_EXODUS_SESSION "Build a session to Exodus-II side sets" ON
  SMTK_ENABLE_VTK_SUPPORT OFF)
cmake_dependent_option(SMTK_ENABLE_REMOTE_SESSION "Build a session that allows modeling in a remote process" ON
  SMTK_ENABLE_REMUS_SUPPORT OFF)

option(SMTK_USE_SYSTEM_MOAB "Use the system-installed moab?" OFF)
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
  enable_testing()
  include(CTest)
  # Do not report some warnings from generated code to the dashboard:
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake/CTestCustom.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake" COPYONLY)

  # Add a test to check for the copyright statment in all source:
  add_test(NAME CopyrightStatement
    COMMAND ${CMAKE_COMMAND}
        "-DSMTK_SOURCE_DIR=${SMTK_SOURCE_DIR}"
        -P "${SMTK_SOURCE_DIR}/CMake/CheckCopyright.cmake")
endif()

################################################################################
# Boost Related Settings
################################################################################

# Boost is required for boost::uuids::uuid and, depending
# on the platform, for filesystem support.
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
# * String algorithms
# * UUID Generation
find_package(Boost 1.60.0
             COMPONENTS   filesystem system  REQUIRED)

#setup windows exception handling so we can compile properly with boost enabled
if(WIN32 AND MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
  add_definitions(-DBOOST_ALL_NO_LIB)
endif()

################################################################################
# Moab Related Settings
################################################################################

#if we are not using system moab we will build our own version through
#the thirdparty folder
if (SMTK_USE_SYSTEM_MOAB)
  # Moab is required for building smtk::mesh, as it is the default storage
  # format for meshes.
  find_package(MOAB REQUIRED)
endif()


################################################################################
# Qt Related Settings
################################################################################

#make find qt4 bring in import targets which are far nicer to use
if(SMTK_ENABLE_QT_SUPPORT)
  set(QT_USE_IMPORTED_TARGETS TRUE)
  find_package(Qt4 REQUIRED)
endif()

################################################################################
# NOTE: This has be before any find_package(VTK), because currently ParaView
# and VTK can NOT be easily switched due to modules and macros intermixed
################################################################################
################################################################################
# Look for Discrete session discrete model modules
################################################################################

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  # include ParaView and ensure it is using the OpenGL2 backend
  find_package(ParaView)
endif()

################################################################################
# VTK Related Settings
################################################################################

if(SMTK_ENABLE_VTK_SUPPORT AND NOT SMTK_ENABLE_PARAVIEW_SUPPORT)
  # Find the package here so environment variables are set, but
  # be careful not to include the VTK_USE_FILE until inside
  # a subdirectory that requires it; otherwise the list of include
  # paths can become too long for some platforms.
  find_package(VTK REQUIRED)
endif()

if (SMTK_ENABLE_PARAVIEW_SUPPORT OR SMTK_ENABLE_VTK_SUPPORT)
  if(NOT VTK_RENDERING_BACKEND STREQUAL "OpenGL2")
    message(FATAL_ERROR
      "SMTK requires that VTK be built with the OpenGL2 Rendering backend.")
  endif()
endif()

################################################################################
# CGM Related Settings
################################################################################

if(SMTK_ENABLE_CGM_SESSION)
  find_package(CGM REQUIRED)
endif(SMTK_ENABLE_CGM_SESSION)

################################################################################
# Remus related settings
################################################################################

if (SMTK_ENABLE_REMUS_SUPPORT)
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
if(SMTK_ENABLE_PYTHON_WRAPPING)
  include(UseShiboken)
  find_package(PythonInterp 2.7 REQUIRED)
  # Set up environment variables needed to import the python modules for tests.
  if (PYTHONINTERP_FOUND AND SMTK_ENABLE_TESTING)
    get_filename_component(SHIBOKEN_LIBRARY_PATH ${SHIBOKEN_LIBRARY} PATH)
    set(SMTK_TEST_LIB_ENV "")
    if(APPLE AND UNIX)
      set(SMTK_TEST_LIB_ENV "DYLD_FALLBACK_LIBRARY_PATH=${SHIBOKEN_LIBRARY_PATH}:${LIBRARY_OUTPUT_PATH}")
      if (ParaView_DIR)
        set(VTKPY_DIR "${ParaView_DIR}/lib/site-packages:${ParaView_DIR}/lib:")
      elseif (VTK_DIR)
        set(VTKPY_DIR "${VTK_DIR}/lib/site-packages:${VTK_DIR}/lib:")
      endif()
    elseif(UNIX)
      if (ParaView_DIR)
        # Note that ending with a colon should only be done if followed by
        # another path... be careful when changing SMTK_TEST_LIB_ENV!
        set(PV_LIB_DIR "${ParaView_DIR}/lib:")
        set(VTKPY_DIR "${ParaView_DIR}/lib/site-packages:${PV_LIB_DIR}")
      elseif (VTK_DIR)
        set(PV_LIB_DIR "${VTK_DIR}/lib:")
        set(VTKPY_DIR "${VTK_DIR}/lib/site-packages:${PV_LIB_DIR}")
      endif()
      set(SMTK_TEST_LIB_ENV "LD_LIBRARY_PATH=${PV_LIB_DIR}${SHIBOKEN_LIBRARY_PATH}:${LIBRARY_OUTPUT_PATH}")
    endif()
    set(SHIBOKEN_SMTK_PYTHON "${CMAKE_BINARY_DIR}")
    if(UNIX)
      set(sep ":")
    else() #windows
      set(sep ";")
    endif()
    execute_process(
      COMMAND "${PYTHON_EXECUTABLE}" -c "import shiboken"
      OUTPUT_QUIET
      ERROR_QUIET
      RESULT_VARIABLE rc)
    if (rc)
      set(SHIBOKEN_SMTK_PYTHON "${SHIBOKEN_SMTK_PYTHON}${sep}${SHIBOKEN_PYTHONPATH}")
    endif ()
    set(SMTK_BINARY_TEST_PATHS "${CMAKE_BINARY_DIR}/lib${sep}${PROJECT_BINARY_DIR}/lib${sep}${PROJECT_BINARY_DIR}${sep}")
    set(SMTK_PYTHONPATH "PYTHONPATH=${SMTK_BINARY_TEST_PATHS}${VTKPY_DIR}${SHIBOKEN_SMTK_PYTHON}")
  endif()
  #message("${SMTK_TEST_LIB_ENV}   ${SMTK_PYTHONPATH}")
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
  DESTINATION include/smtk/${SMTK_VERSION}/smtk)

################################################################################
# Determine hash<X> specialization
################################################################################
find_hash_functor(SMTK_HASH_H SMTK_HASH_NS SMTK_HASH_BEGIN_NS SMTK_HASH_END_NS SMTK_HASH_SPECIALIZATION)
if (NOT SMTK_HASH_H)
  message(FATAL_ERROR "Failed to find a header which provides std::hash<> (or equivalent).")
endif ()
configure_file(
  ${PROJECT_SOURCE_DIR}/CMake/HashFunctor.h.in
  ${PROJECT_BINARY_DIR}/smtk/HashFunctor.h
  @ONLY)

install (FILES ${PROJECT_BINARY_DIR}/smtk/HashFunctor.h
  DESTINATION include/smtk/${SMTK_VERSION}/smtk)

find_file(CGM_HAVE_VERSION_H
  NAMES cgm_version.h
  PATHS ${CGM_INCLUDE_DIRS}
  DOC   "Location of CGM's cgm_version.h")

################################################################################
# Save compile-time options for use by other packages
################################################################################
configure_file(
  ${PROJECT_SOURCE_DIR}/CMake/Options.h.in
  ${PROJECT_BINARY_DIR}/smtk/Options.h
  @ONLY)

install (FILES ${PROJECT_BINARY_DIR}/smtk/Options.h
  DESTINATION include/smtk/${SMTK_VERSION}/smtk)


################################################################################
# Install Related Settings
################################################################################

# Install rules for SMTK macros usable by external packages:
install(
  FILES
    ${PROJECT_SOURCE_DIR}/CMake/EncodeCStringFunctions.cmake
    ${PROJECT_SOURCE_DIR}/CMake/SMTKOperatorXML.cmake
    ${PROJECT_SOURCE_DIR}/CMake/SMTKSessionJSON.cmake
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/SMTK
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
)
include_directories(
  SYSTEM
  "${SPARSEHASH_INCLUDE_DIRS}"
  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cJSON"
  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml"
  "${Boost_INCLUDE_DIRS}"
)

################################################################################
# Finally build smtk
################################################################################

# On Mac OS X, set the dir. included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif ()

add_subdirectory(smtk)

################################################################################
# Install a package targets file
#
# Setup the exports for the library when used from an installed location
install(
  EXPORT SMTK
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SMTK
  FILE SMTKTargets.cmake
)

export(EXPORT SMTK FILE "${PROJECT_BINARY_DIR}/SMTKTargets.cmake")

################################################################################
# Create a build directory package configuration file
#
# Setup the config file for exports that stores what other thirdparty
# packages we need to search for ( MOAB, Remus, etc ) for when using the
# build version of smtk
#
# Required to happen before we include the docs directory
set(MOAB_ROOT_DIR "${MOAB_ROOT_DIR}")
set(SMTK_VTK_MODULE_DIR "${CMAKE_BINARY_DIR}/lib/cmake/SMTK")
set(SMTK_MODULE_DIR "${PROJECT_SOURCE_DIR}/CMake")
set(SMTK_CONFIG_DIR "${PROJECT_BINARY_DIR}")
configure_file(
  ${PROJECT_SOURCE_DIR}/CMake/SMTKConfig.cmake.in
  ${PROJECT_BINARY_DIR}/SMTKConfig.cmake
  @ONLY)

################################################################################
# Create an install package configuration file
#
# Setup the config file for exports that stores what other thirdparty
# packages we need to search for ( MOAB, Remus, etc ) for when using the
# install version of smtk
set(SMTK_VTK_MODULE_DIR "\${CMAKE_CURRENT_LIST_DIR}")
set(SMTK_MODULE_DIR "\${CMAKE_CURRENT_LIST_DIR}")
set(SMTK_CONFIG_DIR "\${CMAKE_CURRENT_LIST_DIR}")
configure_file(
  ${PROJECT_SOURCE_DIR}/CMake/SMTKConfig.cmake.in
  ${PROJECT_BINARY_DIR}/CMakeFiles/SMTKConfig.cmake
  @ONLY)

install (FILES ${PROJECT_BINARY_DIR}/CMakeFiles/SMTKConfig.cmake
         DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SMTK)

################################################################################
# Build documentation
# This also includes tutorials and other documentation that has its
# source checked against SMTK libraries, so it must come after those
# targets have been declared.
################################################################################

if (SMTK_ENABLE_DOCUMENTATION)
  add_subdirectory(doc)
endif()

cmake_minimum_required(VERSION 3.20)

#If the user/superbuild hasn't explicitly stated what c++ standard to use
#require C++11
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED True)
  set(CMAKE_CXX_EXTENSIONS FALSE)
endif()

################################################################################
# SMTK version number
include(CMake/smtkVersion.cmake)
project(smtk VERSION ${SMTK_VERSION})

#Add our Cmake directory to the module search path
set (smtk_cmake_dir ${PROJECT_SOURCE_DIR}/CMake)
list(APPEND CMAKE_MODULE_PATH ${smtk_cmake_dir})

################################################################################

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

# Include cmake code that generates and installs smtk plugins
include(SMTKPluginMacros)

# include cmake code that allows for easier testing of SMTK classes
# and also provides helpers around create_test_sourcelist
include(CMake/SMTKTestingMacros.cmake)

# Add options for performing code coverage tests.
include(CMake/SMTKCoverage.cmake)

# Add options for performing sanitization.
include(CMake/SMTKSanitize.cmake)

# Include mechanism for determining function and bind support
include(Function)

# Set up rpaths in the install so that libraries all reference each other using
# their current location.
if (APPLE)
  list(APPEND CMAKE_INSTALL_RPATH
    "@loader_path")
elseif (UNIX)
  list(APPEND CMAKE_INSTALL_RPATH
    "$ORIGIN")
endif ()

################################################################################
## Paths for output

# Force LIBRARY_OUTPUT_PATH to be a cache variable, whether it was already defined or not.
if (NOT DEFINED LIBRARY_OUTPUT_PATH)
  set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR} CACHE INTERNAL "Directory for all libraries.")
else()
  set(LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH} CACHE INTERNAL "Directory for all libraries.")
endif()
# Set the directory where the binaries will be stored
set(EXECUTABLE_OUTPUT_PATH         ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

# Set the directory where cmake configuration files are installed. The
# convention for this directory's location is OS-dependent. See
# https://cmake.org/cmake/help/latest/command/find_package.html#search-procedure
# for more information.
set(SMTK_INSTALL_CONFIG_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${SMTK_VERSION})

# If SMTK_INSTALL_TESTING_DATA is set to ON, set the directory where smtk's data
# directory is installed.
if (SMTK_INSTALL_TESTING_DATA)
  set(SMTK_INSTALL_DATA_DIR share/${PROJECT_NAME}/${SMTK_VERSION})
endif()

################################################################################
# Options that the user controls
################################################################################
include(CMakeDependentOption)
option(BUILD_SHARED_LIBS "Build SMTK using shared libraries" ON)
option(SMTK_NO_SYSTEM_BOOST "Allow boost to search for system installed boost" ON)
# Option to build Qt ui compoments for attributes
option(SMTK_ENABLE_TESTING "Enable Testing" ON)
cmake_dependent_option(SMTK_ENABLE_UNSTABLE_TESTS "Enable Unstable Testing" OFF SMTK_ENABLE_TESTING OFF)
option(SMTK_INSTALL_TESTING_DATA "Install Testing Data" OFF)
cmake_dependent_option(SMTK_ENABLE_EXAMPLES "Enable Examples" OFF
  SMTK_ENABLE_TESTING OFF)
option(SMTK_ENABLE_PYTHON_WRAPPING "Build Python Wrappings" OFF)

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
cmake_dependent_option(SMTK_ENABLE_APPLICATIONS "Build Qt-enabled applications" ON SMTK_ENABLE_QT_SUPPORT OFF)
cmake_dependent_option(
  SMTK_ENABLE_OPERATION_THREADS "Run operations in asynchronous threads" ON SMTK_ENABLE_QT_SUPPORT OFF)
mark_as_advanced(SMTK_ENABLE_OPERATION_THREADS)
option(SMTK_ENABLE_VTK_SUPPORT "Build VTK component" ON)
option(SMTK_ENABLE_REMUS_SUPPORT "Build Remus components" OFF)
cmake_dependent_option(SMTK_ENABLE_PARAVIEW_SUPPORT "Build paraview plugins for model sessions" OFF
  "SMTK_ENABLE_VTK_SUPPORT;SMTK_ENABLE_QT_SUPPORT" OFF)

option(SMTK_ENABLE_POLYGON_SESSION "Build Boost.polygon model session." ON)
cmake_dependent_option(SMTK_ENABLE_VTK_SESSION "Build a session that uses VTK multiblock datasets" ON
  SMTK_ENABLE_VTK_SUPPORT OFF)
option(SMTK_ENABLE_MESH_SESSION
  "Build a session that uses smtk::mesh as its backing" ON)
cmake_dependent_option(SMTK_ENABLE_MULTISCALE_SESSION "Build a session to Multiscale data" OFF
  "SMTK_ENABLE_VTK_SUPPORT;SMTK_ENABLE_MESH_SESSION;SMTK_ENABLE_PYTHON_WRAPPING" OFF)
if (SMTK_ENABLE_MULTISCALE_SESSION)
  set(AFRL_DIR "/path/to/afrl/data" CACHE PATH "Path to a directory of AFRL data.")
endif()
option(SMTK_ENABLE_OSCILLATOR_SESSION "Build example oscillator model session." ON)

# When SMTK_RELOCATABLE_INSTALL=OFF we add to smtkConfig.cmake
# set(SMTK_PREFIX_PATH "${CMAKE_PREFIX_PATH}")
option(SMTK_RELOCATABLE_INSTALL
  "Do not embed hard-coded paths into the install" ON)
mark_as_advanced(SMTK_RELOCATABLE_INSTALL)

option(SMTK_BUILD_PRECOMPILED_HEADERS
  "Precompile headers to improve build times" OFF)
mark_as_advanced(SMTK_BUILD_PRECOMPILED_HEADERS)

option(SMTK_ENABLE_GRAPHVIZ_SUPPORT "Use graphviz for task layout." OFF)
if (SMTK_ENABLE_GRAPHVIZ_SUPPORT)
  find_package(Graphviz REQUIRED)
endif()

option(SMTK_ENABLE_GDAL_SUPPORT "Include I/O for GDAL. This requires ParaView/VTK built with GDAL."
  OFF)

cmake_dependent_option(SMTK_ENABLE_MARKUP
  "Build the markup annotation library" ON
  SMTK_ENABLE_VTK_SUPPORT OFF)

option(SMTK_USE_BOOST_REGEX "Use Boost::Regex instead of STL" OFF)
mark_as_advanced(SMTK_USE_BOOST_REGEX)

# SMTK_BUILD_DOCUMENTATION is an enumerated option:
# never  == No documentation, and no documentation tools are required.
# manual == Only build when requested; documentation tools (doxygen and
#           sphinx) must be located during configuration.
# always == Build documentation as part of the default target; documentation
#           tools are required. This is useful for automated builds that
#           need "make; make install" to work, since installation will fail
#           if no documentation is built.
set(SMTK_BUILD_DOCUMENTATION
  "never" CACHE STRING "When to build Doxygen- and Sphinx-generated documentation.")
set_property(CACHE SMTK_BUILD_DOCUMENTATION PROPERTY STRINGS never manual always)
if (NOT SMTK_BUILD_DOCUMENTATION STREQUAL "never")
  find_package(Doxygen)
  find_package(Sphinx)
endif()

################################################################################
# Testing Related Settings
################################################################################

#turn on ctest if we want testing
if (SMTK_ENABLE_TESTING)
  set(SMTK_PUBLIC_DROP_SITE ON CACHE BOOL "Submit test results to public dashboards.")
  mark_as_advanced(SMTK_PUBLIC_DROP_SITE)

  enable_testing()
  include(CTest)

  set(SMTK_PLUGIN_CONTRACT_FILE_URLS ""
    CACHE STRING "Local CMake file describing plugins as external projects for contract testing.")

  # ++ contract-example ++
  if (SMTK_ENABLE_PARAVIEW_SUPPORT)
    set(local_url_prefix "file://")
    if (WIN32)
      string(APPEND local_url_prefix "/")
    endif()
    list(APPEND SMTK_PLUGIN_CONTRACT_FILE_URLS
      # Example
      # "${local_url_prefix}${CMAKE_CURRENT_SOURCE_DIR}/CMake/resource-manager-state.cmake"
      )
  endif()
  # -- contract-example --

  # Build an example application (Windows is not yet supported)
  if (NOT WIN32)
    if (SMTK_ENABLE_EXAMPLES AND SMTK_ENABLE_MESH_SESSION AND
        SMTK_ENABLE_VTK_SUPPORT AND SMTK_ENABLE_QT_SUPPORT)
      configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/applications/ModelViewer/model-viewer.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/applications/ModelViewer/model-viewer.cmake"
        @ONLY)
      list(APPEND SMTK_PLUGIN_CONTRACT_FILE_URLS
        "file://${CMAKE_CURRENT_BINARY_DIR}/applications/ModelViewer/model-viewer.cmake")
    endif()
  endif()

  set(SMTK_COVERAGE_ENABLED OFF)
  if(SMTK_ENABLE_COVERAGE)
    set(SMTK_COVERAGE_ENABLED ON)
  endif()

  # Do not report some warnings from generated code to the dashboard:
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake/CTestCustom.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake")

  # Add a test to check for the copyright statment in all source:
  add_test(NAME CopyrightStatement
    COMMAND ${CMAKE_COMMAND}
        "-Dsmtk_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
        -P "${smtk_SOURCE_DIR}/CMake/CheckCopyright.cmake")
endif()

if(SMTK_ENABLE_TESTING OR SMTK_INSTALL_TESTING_DATA)
  unset(SMTK_DATA_DIR CACHE)
  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/data/smtk-data" data)
  if (NOT data STREQUAL "\n")
    if (NOT DEFINED ENV{GITLAB_CI})
      message(WARNING
        "Testing data is not available. Use git-lfs in order to obtain the "
        "testing data.")
    endif()
    set(SMTK_DATA_DIR)
  else ()
    set(SMTK_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")
  endif ()
endif()

# On some *nix distributions, RPath has been deprecated in favor for RunPath. One
# difference between these concepts is that RunPath is not inherited. This causes
# issues with locating libraries without manually setting LD_LIBRARY_PATH when the
# indirect dependency is in a nonstandard location. This flag switches the compiler
# to use RPath in favor of RunPath.
cmake_dependent_option(SMTK_FORCE_OLD_DTAGS "Use deprecated RPath in favor of RunPath"
  OFF "UNIX;NOT APPLE" OFF)
if (SMTK_FORCE_OLD_DTAGS)
  string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--disable-new-dtags")
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
# * Date Time
# * Filesystem
# * String algorithms
# * UUID Generation
set(required_boost_components
  date_time filesystem system)

# Some c++ compilers do not support regex, so we may need Boost's regex library.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
    (CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND
      (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9.0")) OR
    (WIN32 AND MSVC))
#we definitely do not need regex support
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9.0")
#we definitely need regex support
  set(SMTK_USE_BOOST_REGEX ON)
else()
#we may need regex support, but rather than try-compile let's just use boost
  set(SMTK_USE_BOOST_REGEX ON)
endif()

# If SMTK wants/needs to use Boost::regex then add it to the list
if(SMTK_USE_BOOST_REGEX)
# we want regex support, we don't care if the compiler provides it or not
  list(APPEND required_boost_components regex)
endif()

set(SMTK_MINIMUM_BOOST_VERSION 1.64.0)
find_package(Boost ${SMTK_MINIMUM_BOOST_VERSION}
             COMPONENTS ${required_boost_components} REQUIRED)

if(WIN32 AND MSVC)
  #setup windows exception handling so we can compile properly with boost
  #enabled
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
  #quiet warnings about printf being potentially unsafe
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")
  #quiet warnings about truncating decorated name
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4503")
  #resolve issue with number of sections exceeding object file format limit when compiling in debug.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  add_definitions(-DBOOST_ALL_NO_LIB)
endif()

find_package(Eigen3 REQUIRED)

# Setting this ensures that "make install" will leave rpaths to external
# libraries (not part of the build-tree e.g. Qt, ffmpeg, etc.) intact on
# "make install". This ensures that one can install a version of ParaView on the
# build machine without any issues. If this not desired, simply specify
# CMAKE_INSTALL_RPATH_USE_LINK_PATH when configuring and "make install" will
# strip all rpaths, which is default behavior.
if (NOT CMAKE_INSTALL_RPATH_USE_LINK_PATH)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif ()

################################################################################
# LibArchive Related Settings
################################################################################
find_package(LibArchive REQUIRED)

################################################################################
# JSON Related Settings
################################################################################
find_package(nlohmann_json REQUIRED)

################################################################################
# PEGTL Related Settings
################################################################################
find_package(pegtl REQUIRED)

################################################################################
# units Related Settings
################################################################################
find_package(units REQUIRED)

################################################################################
# fmt Related Settings
################################################################################
find_package(fmt REQUIRED)

################################################################################
# Moab Related Settings
################################################################################

# Moab is required for building smtk::mesh, as it is the default storage
# format for meshes.
find_package(MOAB REQUIRED)


################################################################################
# Qt Related Settings
################################################################################

################################################################################
# NOTE: This has be before any find_package(VTK), because currently ParaView
# and VTK can NOT be easily switched due to modules and macros intermixed
################################################################################

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  # include ParaView and ensure it is using the OpenGL2 backend
  find_package(ParaView REQUIRED)
  if (SMTK_ENABLE_PYTHON_SUPPORT AND NOT PARAVIEW_ENABLE_PYTHON)
    message(FATAL_ERROR
      "SMTK with Python and ParaView support requires that ParaView also be "
      "built with Python support.")
  endif ()
endif()

################################################################################
# Thread Related Settings
################################################################################
find_package(Threads REQUIRED)

################################################################################
# VTK Related Settings
################################################################################

if(SMTK_ENABLE_VTK_SUPPORT AND NOT SMTK_ENABLE_PARAVIEW_SUPPORT)
  find_package(VTK REQUIRED)
  if (SMTK_ENABLE_PYTHON_SUPPORT AND NOT VTK_WRAP_PYTHON)
    message(FATAL_ERROR
      "SMTK with Python and VTK support requires that VTK also be built with "
      "Python support.")
  endif ()
endif()

################################################################################
# Remus related settings
################################################################################

if (SMTK_ENABLE_REMUS_SUPPORT)
  find_package(Remus REQUIRED)
endif()

################################################################################
# Build utility executables
################################################################################

add_subdirectory(utilities)

################################################################################
# Build third party libraries
################################################################################

add_subdirectory(thirdparty)

################################################################################
# Wrapping Related Settings
################################################################################
if(SMTK_ENABLE_PYTHON_WRAPPING)
  if(WIN32)
    set(PYTHON_MODULE_EXTENSION ".pyd")
  else()
    set(PYTHON_MODULE_EXTENSION ".so")
  endif()

  if (SMTK_PYTHON_VERSION STREQUAL "2")
    message(FATAL_ERROR
      "Python2 is no longer supported.")
  endif ()
  if (SMTK_ENABLE_VTK_SUPPORT OR SMTK_ENABLE_PARAVIEW_SUPPORT)
    if (NOT VTK_PYTHON_VERSION STREQUAL "3")
      message(FATAL_ERROR
        "VTK must be compiled with Python3 support.")
    endif ()
  endif ()
  find_package(Python3 REQUIRED
    COMPONENTS Interpreter Development.Embed Development.Module)

  # Initialize SMTK_PYTHON_MODULEDIR.
  # This stores the location where we'll install SMTK's Python modules.
  # Note that SMTK_PYTHON_MODULEDIR may be provided if SMTK is being
  # built as a submodule or as part of a superbuild.
  if (NOT DEFINED SMTK_PYTHON_MODULEDIR)
    if (SMTK_INSTALL_PYTHON_TO_SITE_PACKAGES)
        execute_process(
          COMMAND
          "$<TARGET_FILE:Python3::Interpreter>"
          -c "import site; print(site.getsitepackages())[-1]"
          RESULT_VARIABLE SMTK_PYTHON_MODULEDIR
          )
    elseif(WIN32)
        set(SMTK_PYTHON_MODULEDIR
          "bin/Lib/site-packages")
    else()
        set(SMTK_PYTHON_MODULEDIR
          "${CMAKE_INSTALL_LIBDIR}/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages")
    endif()
  endif()

  find_package(pybind11 2.3 REQUIRED)
  set(pybind11_VERSION "2.3")

  set(SMTK_PYBIND11_FLAGS " ")
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
     CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
     CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(SMTK_PYBIND11_FLAGS "${SMTK_PYBIND11_FLAGS} -Wno-shadow")
  endif()

  # Set up environment variables needed to import the python modules for tests.
  if (Python3_Interpreter_FOUND AND SMTK_ENABLE_TESTING)
    set(required_python_modules)

    # Add the path to the build tree's compiled modules, prioritizing it over
    # the other directories (since the other directories will likely include the
    # location of the module's install).
    set(smtk_pythonpaths
      "${CMAKE_BINARY_DIR}/${SMTK_PYTHON_MODULEDIR}")
    set(smtk_libpaths
      "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")

    if (SMTK_ENABLE_PARAVIEW_SUPPORT)
      list(APPEND smtk_pythonpaths
        "${ParaView_PREFIX_PATH}/${PARAVIEW_PYTHONPATH}")
      list(APPEND smtk_libpaths
        "${ParaView_PREFIX_PATH}/${CMAKE_INSTALL_BINDIR}")
    elseif (SMTK_ENABLE_VTK_SUPPORT)
      list(APPEND smtk_pythonpaths
        "${VTK_PREFIX_PATH}/${VTK_PYTHONPATH}")
      list(APPEND smtk_libpaths
        "${VTK_PREFIX_PATH}/${CMAKE_INSTALL_BINDIR}")
    endif ()

    set(envsep ":")
    if (WIN32)
      set(envsep ";")
    endif ()

    set(libpath_envvar)
    if (WIN32)
      list(INSERT smtk_libpaths 0
        $ENV{PATH})
      set(libpath_envvar "PATH")
    endif ()

    # Remove duplicates.
    if (smtk_pythonpaths)
      list(REMOVE_DUPLICATES smtk_pythonpaths)
    endif ()
    if (smtk_libpaths)
      list(REMOVE_DUPLICATES smtk_libpaths)
    endif ()

    # Append paths as needed.
    list(APPEND smtk_pythonpaths "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    if (libpath_envvar)
      string(APPEND smtk_libpath_env "${envsep}$ENV{${libpath_envvar}}")
    endif ()

    string(REPLACE ";" "${envsep}" smtk_pythonpath_env "${smtk_pythonpaths}")
    string(REPLACE ";" "${envsep}" smtk_libpath_env "${smtk_libpaths}")
    if (WIN32)
      string(REPLACE "/" "\\" smtk_libpath_env "${smtk_libpath_env}")
    endif ()
    string(REPLACE ";" "\;" smtk_pythonpath_env "${smtk_pythonpath_env}")
    string(REPLACE ";" "\;" smtk_libpath_env "${smtk_libpath_env}")
    set(smtk_python_test_environment
      "PYTHONPATH=${smtk_pythonpath_env}")

    if (libpath_envvar AND smtk_libpath_env)
      list(APPEND smtk_python_test_environment
        "${libpath_envvar}=${smtk_libpath_env}")
    endif ()

    function (smtk_add_test_python name file)
      if (NOT IS_ABSOLUTE "${file}")
        set(file "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
      endif ()
      add_test(
        NAME    "${name}"
        COMMAND "$<TARGET_FILE:Python3::Interpreter>" "${file}"
                ${ARGN})
      set_tests_properties("${name}"
        PROPERTIES
          ENVIRONMENT "${smtk_python_test_environment}"
          # Note, if you change SKIP_RETURN_CODE, you must also change
          # the value of SKIP_ENTIRE in smtk/smtk/testing.py
          SKIP_RETURN_CODE 42
      )
      smtk_test_apply_sanitizer("${name}")
    endfunction ()
  endif()
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

install (FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/Function.h
  DESTINATION include/${PROJECT_NAME}/${SMTK_VERSION}/smtk)

################################################################################
# Save compile-time options for use by other packages
################################################################################
configure_file(
  ${PROJECT_SOURCE_DIR}/CMake/Options.h.in
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/Options.h
  @ONLY)

install (FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/Options.h
  DESTINATION include/${PROJECT_NAME}/${SMTK_VERSION}/smtk)


################################################################################
# Include Dirs Settings
################################################################################

# Add the current directory so we can include files like: <smtk/internal/Test.h>
# Save the include directories in a variable so that VTK modules (inside the
# extension/vtk directories) can reuse them.
include_directories(
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
include_directories(
  SYSTEM
  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty"
  "${CMAKE_CURRENT_BINARY_DIR}/thirdparty"
  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml"
  "${Boost_INCLUDE_DIRS}"
)

################################################################################
# Finally build smtk
################################################################################

# On Mac OS X, set the dir. included as part of the installed library's path:
if (BUILD_SHARED_LIBS AND NOT DEFINED CMAKE_INSTALL_NAME_DIR)
  set(CMAKE_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif ()

add_subdirectory(smtk)

# Build SMTK Applications
if(SMTK_ENABLE_APPLICATIONS)
  add_subdirectory(applications)
endif()

################################################################################
# Install a package targets file
#
# Setup the exports for the library when used from an installed location
install(
  EXPORT ${PROJECT_NAME}
  DESTINATION ${SMTK_INSTALL_CONFIG_DIR}
  FILE ${PROJECT_NAME}Targets.cmake
)

export(EXPORT ${PROJECT_NAME} FILE "${PROJECT_BINARY_DIR}/${SMTK_INSTALL_CONFIG_DIR}/${PROJECT_NAME}Targets.cmake")

################################################################################
# Create a build directory package configuration file
#
# Setup the config file for exports that stores what other thirdparty
# packages we need to search for ( MOAB, Remus, etc ) for when using the
# build version of smtk
#
# Required to happen before we include the docs directory

if (SMTK_ENABLE_PYTHON_WRAPPING)
  set(smtk_python_executable ${PYTHON_EXECUTABLE})
  set(smtk_python_library ${PYTHON_LIBRARIES})
  set(smtk_python_include_dir ${PYTHON_INCLUDE_DIRS})
  set(smtk_python_moduledir)
endif ()

if (SMTK_DATA_DIR)
  set(smtk_data_dir ${SMTK_DATA_DIR})
endif()

set(smtk_cmake_dir "${CMAKE_CURRENT_LIST_DIR}/CMake")
set(smtk_cmake_build_dir "${CMAKE_BINARY_DIR}/${SMTK_INSTALL_CONFIG_DIR}")
set(smtk_cmake_destination "${SMTK_INSTALL_CONFIG_DIR}")
include(SMTKInstallCMakePackage)

set(SMTK_VTK_MODULE_DIR "${CMAKE_BINARY_DIR}/lib/cmake/${PROJECT_NAME}/${SMTK_VERSION}")
set(SMTK_MODULE_DIR "${PROJECT_SOURCE_DIR}/CMake")
set(SMTK_CONFIG_DIR "${PROJECT_BINARY_DIR}")
get_property(SMTK_PLUGINS GLOBAL PROPERTY SMTK_PLUGINS)
configure_file(
  ${PROJECT_SOURCE_DIR}/CMake/${PROJECT_NAME}Config.cmake.in
  ${SMTK_INSTALL_CONFIG_DIR}/${PROJECT_NAME}Config.cmake
  @ONLY)

################################################################################
# Create an install package configuration file
#
# Setup the config file for exports that stores what other thirdparty
# packages we need to search for ( MOAB, Remus, etc ) for when using the
# install version of smtk

# If we are performing a relocatable install, we must erase the hard-coded
# install paths we set in smtk_prefix_path before constructing the install
# package configuration file
if (SMTK_RELOCATABLE_INSTALL)
  set(smtk_prefix_path)
endif()

if (SMTK_INSTALL_TESTING_DATA)
  set(smtk_data_dir ${SMTK_INSTALL_DATA_DIR}/data)
else()
  set(smtk_data_dir)
endif()

set(SMTK_VTK_MODULE_DIR "\${CMAKE_CURRENT_LIST_DIR}")
set(SMTK_MODULE_DIR "\${CMAKE_CURRENT_LIST_DIR}")
set(SMTK_CONFIG_DIR "\${CMAKE_CURRENT_LIST_DIR}")
configure_file(
  ${PROJECT_SOURCE_DIR}/CMake/${PROJECT_NAME}Config.cmake.in
  ${PROJECT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake
  @ONLY)

install (FILES ${PROJECT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake
         DESTINATION ${SMTK_INSTALL_CONFIG_DIR})

################################################################################
# Build documentation
# This also includes tutorials and other documentation that has its
# source checked against SMTK libraries, so it must come after those
# targets have been declared.
################################################################################

if (NOT SMTK_BUILD_DOCUMENTATION STREQUAL "never")
  add_subdirectory(doc)
endif()

################################################################################
# Test plugins
#
# By adding a plugin contract file URL to SMTK_PLUGIN_CONTRACT_FILE_URLS, the
# user can construct a list of plugins that will be built  and tested along with
# smtk's unit tests. See <PROJECT_SOURCE_DIR>/CMake/resource-manager-state.cmake
# for an example of a plugin contract file.
################################################################################

if (SMTK_ENABLE_TESTING AND SMTK_PLUGIN_CONTRACT_FILE_URLS AND
    NOT SMTK_ENABLE_SANITIZER AND NOT SMTK_ENABLE_COVERAGE)
  include(CMake/SMTKPluginTestingMacros.cmake)

  list(REMOVE_DUPLICATES SMTK_PLUGIN_CONTRACT_FILE_URLS)
  foreach(plugin_url ${SMTK_PLUGIN_CONTRACT_FILE_URLS})
    smtk_test_plugin(${plugin_url})
  endforeach()
endif()

if (SMTK_INSTALL_TESTING_DATA AND SMTK_DATA_DIR)
  install(DIRECTORY ${SMTK_DATA_DIR} DESTINATION ${SMTK_INSTALL_DATA_DIR})
endif()

################################################################################
# Each subdirectory exports variables naming header and source files
# to its parent scope (this CMakeLists.txt) which we combine into
# the smtkCore library.

add_subdirectory(common)
smtk_source_group(common)

add_subdirectory(string)
smtk_source_group(string)

add_subdirectory(resource)
smtk_source_group(resource)

add_subdirectory(geometry)
smtk_source_group(geometry)

add_subdirectory(graph)
smtk_source_group(graph)

add_subdirectory(attribute)
smtk_source_group(attribute)

add_subdirectory(operation)
smtk_source_group(operation)

add_subdirectory(project)
smtk_source_group(project)

add_subdirectory(mesh)
smtk_source_group(mesh)

add_subdirectory(model)
smtk_source_group(model)

add_subdirectory(simulation)
smtk_source_group(simulation)

add_subdirectory(task)
smtk_source_group(task)

add_subdirectory(io)
smtk_source_group(io)

add_subdirectory(view)
smtk_source_group(view)

add_subdirectory(workflow)
smtk_source_group(workflow)

add_subdirectory(plugin)
smtk_source_group(plugin)

set(smtk_headers
  ${commonHeaders}
  ${stringHeaders}
  ${resourceHeaders}
  ${geometryHeaders}
  ${graphHeaders}
  ${attributeHeaders}
  ${operationHeaders}
  ${meshHeaders}
  ${modelHeaders}
  ${projectHeaders}
  ${viewHeaders}
  ${workflowHeaders}
  ${pluginHeaders}
  ${simulationHeaders}
  ${taskHeaders}
  ${ioHeaders}
)
set(smtk_srcs
  ${commonSrcs}
  ${stringSrcs}
  ${resourceSrcs}
  ${geometrySrcs}
  ${graphSrcs}
  ${attributeSrcs}
  ${operationSrcs}
  ${projectSrcs}
  ${meshSrcs}
  ${modelSrcs}
  ${viewSrcs}
  ${workflowSrcs}
  ${pluginSrcs}
  ${simulationSrcs}
  ${taskSrcs}
  ${ioSrcs}
)

set(smtkCore_public_include_directories
  $<BUILD_INTERFACE:${smtk_SOURCE_DIR}>
  $<BUILD_INTERFACE:${smtk_BINARY_DIR}>
  $<INSTALL_INTERFACE:include/smtk/${SMTK_VERSION}>
)

set(smtkCore_public_link_libraries
  MOAB
  nlohmann_json
  taocpp::pegtl
  Threads::Threads
  Boost::boost
  units
)

set(smtkCore_private_link_libraries
  Boost::date_time
  Boost::filesystem
  Boost::system
  LibArchive::LibArchive
  Eigen3::Eigen
  fmt::fmt
)

if(SMTK_ENABLE_PYTHON_WRAPPING)
  set_property(SOURCE common/PythonInterpreter.cxx APPEND PROPERTY COMPILE_DEFINITIONS
    "PYTHON_MODULEDIR=\"${SMTK_PYTHON_MODULEDIR}\"")

  # For code to optionally include ImportPythonOperator
  add_definitions(-DSMTK_PYTHON_ENABLED)

  list(APPEND smtkCore_private_link_libraries
    pybind11::embed
    )
  add_subdirectory(pybind11)
endif()

add_library(smtkCore ${smtk_srcs})
# dependencies on generated files from subdirectories
add_dependencies(smtkCore
  attributeGenHeaders
  graphGenHeaders
  meshGenHeaders
  modelGenHeaders
  operationGenHeaders
  projectGenHeaders
  viewGenHeaders
)

target_include_directories(smtkCore
  PUBLIC
  ${smtkCore_public_include_directories}
)
target_link_libraries(smtkCore
  LINK_PUBLIC ${smtkCore_public_link_libraries}
  LINK_PRIVATE ${smtkCore_private_link_libraries})

smtk_precompile_headers(smtkCore
  PUBLIC
    common/Properties.h
    common/Links.h
    common/Observers.h
    common/TypeName.h
    TupleTraits.h
    )

if (WIN32)
  # Boost 1.70 and above have CMake configuration provided by upstream. Usage
  # requirements are not quite resolved yet.
  # https://github.com/boostorg/boost_install/issues/18
  target_compile_definitions(smtkCore
    PRIVATE
      BOOST_UUID_FORCE_AUTO_LINK)
endif ()

smtk_export_header(smtkCore CoreExports.h)
if (NOT BUILD_SHARED_LIBS AND SMTK_ENABLE_PYTHON_WRAPPING)
  # Set position-independent flag when other project libs are shared.
  set_target_properties(smtkCore PROPERTIES POSITION_INDEPENDENT_CODE True)
endif()

# On Mac OS X, link to the Foundation framework
if (APPLE)
  find_library(SMTK_MACOS_FOUNDATION_FMWK
    NAMES Foundation
    PATHS ${CMAKE_OSX_SYSROOT}/System/Library
    PATH_SUFFIXES Frameworks
    NO_DEFAULT_PATH)
  if (SMTK_MACOS_FOUNDATION_FMWK)
    target_link_libraries(smtkCore
      LINK_PRIVATE ${SMTK_MACOS_FOUNDATION_FMWK})
  endif()
  mark_as_advanced(SMTK_MACOS_FOUNDATION_FMWK)
endif()

if (SMTK_USE_BOOST_REGEX)
  target_link_libraries(smtkCore
    LINK_PUBLIC Boost::regex)
  target_compile_definitions(smtkCore PUBLIC USE_BOOST_REGEX)
endif ()

#install the library and exports the library when used from a build tree
smtk_install_library(smtkCore)

################################################################################
# setup install rules
################################################################################
install(
  FILES
    AutoInit.h
    PublicPointerDefs.h
    Regex.h
    SharedFromThis.h
    SharedPtr.h
    SystemConfig.h
    TupleTraits.h
    Metaprogramming.h
  DESTINATION
    include/smtk/${SMTK_VERSION}/smtk
)

################################################################################
# Generate Python wrapping of the smtkCore library

if (SMTK_ENABLE_PYTHON_WRAPPING)

  #only install PythonAutoInit.h if pybind11 is enabled, since it directly
  #includes pybind11 headers
  install(
    FILES
      PythonAutoInit.h
    DESTINATION
      include/smtk/${SMTK_VERSION}/smtk
    )

  set(SMTK_PYTHON_MODULE_SRCS
    attribute_builder
    testing
    )
  foreach(pyfile ${SMTK_PYTHON_MODULE_SRCS})
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/smtk/${pyfile}.py"
      "${CMAKE_BINARY_DIR}/${SMTK_PYTHON_MODULEDIR}/smtk/${pyfile}.py" @ONLY
      )
    list(APPEND SMTK_PYTHON_MODULEFILES "smtk/${pyfile}.py")
  endforeach()
    install(
      FILES ${SMTK_PYTHON_MODULEFILES}
      DESTINATION "${SMTK_PYTHON_MODULEDIR}/smtk"
      )

  # This variable needs to be set for vtk_module to find its python module
  # output dir
  #
  # TODO: these variables must be treated differently when building with
  # VisualStudio; see VTK's treatment of these variables for the correct
  # handling of that case.
  set(VTK_BUILD_PYTHON_MODULES_DIR ${SMTK_PYTHON_MODULEDIR})
  set(VTK_INSTALL_PYTHON_MODULES_DIR ${SMTK_PYTHON_MODULEDIR})
endif()

################################################################################
# Build smtk extensions
# Add libraries that extend other projects to have support for SMTK.
# For example extension/vtk allows you to show smtk models in VTK.
################################################################################

## Markup library
set(SMTK_MARKUP_SUPPORT_ENABLED "False")
if ((SMTK_ENABLE_PARAVIEW_SUPPORT OR SMTK_ENABLE_VTK_SUPPORT) AND SMTK_ENABLE_MARKUP)
  if (TARGET VTK::FiltersParallelDIY2)
    if (ParaView_VERSION VERSION_GREATER_EQUAL "5.10.0")
      set(SMTK_MARKUP_SUPPORT_ENABLED "True")
      add_subdirectory(markup)
    else()
      message("Markup requires ParaView 5.10.0+. Skipping.")
    endif()
  else()
    message("Markup requires VTK build with FiltersParallelDIY2. Skipping.")
  endif()
endif()
add_subdirectory(extension)

################################################################################
# Build model sessions
# Now that the main components of smtk have been built we can build
# any custom model session sessions that the user has enabled.
################################################################################
add_subdirectory(session)

################################################################################
# Build smtk default plugin initialization
# At this point, all plugins that are built by smtk core have been
# configured. We then aggregate the plugin targets and construct a
# single library for initializing them in a ParaView-derived application.
################################################################################
if (SMTK_ENABLE_PARAVIEW_SUPPORT)
 get_property(smtk_paraview_plugin_files GLOBAL
  PROPERTY _smtk_plugin_files)
paraview_plugin_scan(
  PLUGIN_FILES ${smtk_paraview_plugin_files}
  PROVIDES_PLUGINS paraview_plugins
  ENABLE_BY_DEFAULT ON
  HIDE_PLUGINS_FROM_CACHE ON)
paraview_plugin_build(
  HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${SMTK_VERSION}"
  LIBRARY_SUBDIRECTORY "${PROJECT_NAME}-${SMTK_VERSION}"
  PLUGINS ${paraview_plugins}
  PLUGINS_FILE_NAME "smtk.paraview.xml"
  AUTOLOAD ${paraview_plugins}
  INSTALL_EXPORT SMTKParaViewPlugins
  CMAKE_DESTINATION ${SMTK_INSTALL_CONFIG_DIR}
  ADD_INSTALL_RPATHS ON
  TARGET smtk_paraview_plugins)
endif()

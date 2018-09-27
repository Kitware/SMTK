include(${PARAVIEW_USE_FILE})
include(ParaViewPlugins)

# We need to add the current value of VTK_MODULES_DIR to the module path
# so that when the plugins are built all the modules can be found. Otherwise,
# modules that aren't loaded as direct dependencies of CMB modules will
# not be found.
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

foreach(name ${SMTK_PLUGINS})
  set(SMTK_PLUGIN_IMPORT_INIT "${SMTK_PLUGIN_IMPORT_INIT}PV_PLUGIN_IMPORT_INIT(${name});\n")
  set(SMTK_PLUGIN_IMPORT "${SMTK_PLUGIN_IMPORT}PV_PLUGIN_IMPORT(${name});\n")
  set(SMTK_PLUGIN_QUERY "${SMTK_PLUGIN_QUERY}queryPlugin(${name});\n")
endforeach()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/InitializeDefaultPlugins.cxx.in
  ${CMAKE_CURRENT_BINARY_DIR}/InitializeDefaultPlugins.cxx @ONLY)

vtk_module_dep_includes(vtkPVClientServerCoreCore)
include_directories(${vtkPVClientServerCoreCore_INCLUDE_DIRS}
  ${vtkPVClientServerCoreCore_DEPENDS_INCLUDE_DIRS})
add_library(smtkDefaultPlugins
  ${CMAKE_CURRENT_BINARY_DIR}/InitializeDefaultPlugins.cxx)
target_include_directories(smtkDefaultPlugins
  PUBLIC
  $<BUILD_INTERFACE:${smtk_SOURCE_DIR}>
  $<BUILD_INTERFACE:${smtk_BINARY_DIR}>
  $<INSTALL_INTERFACE:include/smtk/${SMTK_VERSION}>
  )
target_link_libraries(smtkDefaultPlugins
  LINK_PRIVATE ${SMTK_PLUGINS})

smtk_export_header(smtkDefaultPlugins Exports.h)

smtk_public_headers(smtkDefaultPlugins InitializeDefaultPlugins.h)
smtk_install_library(smtkDefaultPlugins)

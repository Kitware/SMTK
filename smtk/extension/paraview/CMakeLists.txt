################################################################################
# Build widgets
################################################################################
if (PARAVIEW_BUILD_QT_GUI)
  add_subdirectory(widgets)
  add_subdirectory(appcomponents)
  add_subdirectory(operators)
endif()
add_subdirectory(pluginsupport)

set(module_files
  "${CMAKE_CURRENT_SOURCE_DIR}/server/vtk.module")
vtk_module_scan(
  MODULE_FILES ${module_files}
  PROVIDES_MODULES vtk_modules
  HIDE_MODULES_FROM_CACHE ON
  WANT_BY_DEFAULT ON)
vtk_module_build(
  MODULES ${vtk_modules}
  PACKAGE SMTKParaViewVTKModules
  INSTALL_EXPORT SMTKParaViewVTKModules
  CMAKE_DESTINATION ${SMTK_INSTALL_CONFIG_DIR}
  TEST_DIRECTORY_NAME "NONE"
  # Handled by SMTK macros. Used in preference to VTK's since SMTK also adds
  # unit tests for each public header.
  INSTALL_HEADERS OFF)

set(plugin_files
  server/plugin/paraview.plugin)

paraview_plugin_scan(
  PLUGIN_FILES ${plugin_files}
  PROVIDES_PLUGINS paraview_plugins
  ENABLE_BY_DEFAULT ON
  HIDE_PLUGINS_FROM_CACHE ON)
paraview_plugin_build(
  HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${PROJECT_VERSION}"
  LIBRARY_SUBDIRECTORY "${PROJECT_NAME}-${PROJECT_VERSION}"
  PLUGINS ${paraview_plugins}
  AUTOLOAD ${paraview_plugins}
  PLUGINS_FILE_NAME "smtk.extensions.xml"
  TARGET smtk_paraview_extension_plugin
  CMAKE_DESTINATION ${SMTK_INSTALL_CONFIG_DIR}
  ADD_INSTALL_RPATHS ON
  INSTALL_EXPORT SMTKParaViewExtensionPlugins)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

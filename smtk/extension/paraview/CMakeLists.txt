################################################################################
# Build widgets
################################################################################
set(module_files
  "${CMAKE_CURRENT_SOURCE_DIR}/appcomponents/vtk.module"
  "${CMAKE_CURRENT_SOURCE_DIR}/mesh/vtk.module"
  "${CMAKE_CURRENT_SOURCE_DIR}/model/vtk.module"
  "${CMAKE_CURRENT_SOURCE_DIR}/server/vtk.module"
)

if (ParaView_VERSION VERSION_GREATER_EQUAL "5.10.0")
  list(APPEND module_files "${CMAKE_CURRENT_SOURCE_DIR}/widgets/vtk.module")
endif()

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
  HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${SMTK_VERSION}"
  TEST_DIRECTORY_NAME "NONE")

set(plugin_files
  server/plugin/paraview.plugin)
if (TARGET ParaView::pqApplicationComponents)
  list(APPEND plugin_files
    appcomponents/plugin-panel-defaults/paraview.plugin
    appcomponents/plugin-legacy-operations/paraview.plugin
    appcomponents/plugin-operations-panel/paraview.plugin
    appcomponents/plugin-core/paraview.plugin
    appcomponents/plugin-gui/paraview.plugin
    appcomponents/plugin-readers/paraview.plugin
    model/plugin/paraview.plugin
    mesh/plugin/paraview.plugin
    operators/paraview.plugin
  )

  if (ParaView_VERSION VERSION_GREATER_EQUAL "5.10.0")
    list(APPEND plugin_files
      widgets/plugin/paraview.plugin
    )
  endif()
endif ()

paraview_plugin_scan(
  PLUGIN_FILES ${plugin_files}
  PROVIDES_PLUGINS paraview_plugins
  ENABLE_BY_DEFAULT ON
  HIDE_PLUGINS_FROM_CACHE ON)
paraview_plugin_build(
  HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${SMTK_VERSION}"
  LIBRARY_SUBDIRECTORY "${PROJECT_NAME}-${SMTK_VERSION}"
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

################################################################################
# Build widgets
################################################################################
set(module_files
  "${CMAKE_CURRENT_SOURCE_DIR}/appcomponents/vtk.module"
  "${CMAKE_CURRENT_SOURCE_DIR}/model/vtk.module"
  "${CMAKE_CURRENT_SOURCE_DIR}/project/vtk.module"
  "${CMAKE_CURRENT_SOURCE_DIR}/server/vtk.module"
)

if (ParaView_VERSION VERSION_GREATER_EQUAL "5.10.0")
  list(APPEND module_files
    "${CMAKE_CURRENT_SOURCE_DIR}/widgets/vtk.module"
    "${CMAKE_CURRENT_SOURCE_DIR}/operators/vtk.module"
  )
endif()
if (SMTK_MARKUP_SUPPORT_ENABLED)
  list(APPEND module_files
    "${CMAKE_CURRENT_SOURCE_DIR}/markup/vtk.module"
  )
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

set(core_plugin_files
  server/plugin/paraview.plugin
)
set(gui_plugin_files)
if (TARGET ParaView::pqApplicationComponents)
  list(APPEND core_plugin_files
    appcomponents/plugin-core/paraview.plugin
    appcomponents/plugin-readers/paraview.plugin
    model/plugin/paraview.plugin
    project/plugin-core/paraview.plugin
  )
  list(APPEND gui_plugin_files
    appcomponents/plugin-panel-defaults/paraview.plugin
    appcomponents/plugin-legacy-operations/paraview.plugin
    appcomponents/plugin-operations-panel/paraview.plugin
    appcomponents/plugin-grouping/paraview.plugin
    appcomponents/plugin-gui/paraview.plugin
    project/plugin-gui/paraview.plugin
  )

  if (ParaView_VERSION VERSION_GREATER_EQUAL "5.10.0")
    list(APPEND core_plugin_files
      widgets/plugin/paraview.plugin
      operators/plugin/paraview.plugin
    )
  endif()
  if (SMTK_MARKUP_SUPPORT_ENABLED)
    list(APPEND core_plugin_files
      markup/plugin/paraview.plugin
    )
  endif()
endif ()

paraview_plugin_scan(
  PLUGIN_FILES ${core_plugin_files}
  PROVIDES_PLUGINS core_paraview_plugins
  ENABLE_BY_DEFAULT ON
  HIDE_PLUGINS_FROM_CACHE ON)
paraview_plugin_build(
  HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${SMTK_VERSION}"
  LIBRARY_SUBDIRECTORY "${PROJECT_NAME}-${SMTK_VERSION}"
  PLUGINS ${core_paraview_plugins}
  AUTOLOAD ${core_paraview_plugins}
  PLUGINS_FILE_NAME "smtk.extensions.core.xml"
  TARGET smtk_paraview_extension_core_plugins
  CMAKE_DESTINATION ${SMTK_INSTALL_CONFIG_DIR}
  ADD_INSTALL_RPATHS ON
  INSTALL_EXPORT SMTKParaViewExtensionCorePlugins)

  paraview_plugin_scan(
  PLUGIN_FILES ${gui_plugin_files}
  PROVIDES_PLUGINS gui_paraview_plugins
  ENABLE_BY_DEFAULT ON
  HIDE_PLUGINS_FROM_CACHE ON)
paraview_plugin_build(
  HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${SMTK_VERSION}"
  LIBRARY_SUBDIRECTORY "${PROJECT_NAME}-${SMTK_VERSION}"
  PLUGINS ${gui_paraview_plugins}
  AUTOLOAD ${gui_paraview_plugins}
  PLUGINS_FILE_NAME "smtk.extensions.gui.xml"
  TARGET smtk_paraview_extension_gui_plugins
  CMAKE_DESTINATION ${SMTK_INSTALL_CONFIG_DIR}
  ADD_INSTALL_RPATHS ON
  INSTALL_EXPORT SMTKParaViewExtensionGuiPlugins)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

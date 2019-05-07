get_property(smtk_paraview_plugin_files GLOBAL
  PROPERTY _smtk_plugin_files)
paraview_plugin_scan(
  PLUGIN_FILES ${smtk_paraview_plugin_files}
  PROVIDES_PLUGINS paraview_plugins
  ENABLE_BY_DEFAULT ON
  HIDE_PLUGINS_FROM_CACHE ON)
paraview_plugin_build(
  HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${PROJECT_VERSION}"
  LIBRARY_SUBDIRECTORY "${PROJECT_NAME}-${PROJECT_VERSION}"
  PLUGINS ${paraview_plugins}
  PLUGINS_FILE_NAME "smtk.paraview.xml"
  AUTOLOAD ${paraview_plugins}
  INSTALL_EXPORT SMTKParaViewPlugins
  CMAKE_DESTINATION ${SMTK_INSTALL_CONFIG_DIR}
  TARGET smtk_paraview_plugins)

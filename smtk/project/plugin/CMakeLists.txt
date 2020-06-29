set(classes
  AutoStart
  pqSMTKProjectBrowser
  pqSMTKProjectMenu
  pqSMTKProjectPanel
  Registrar
)

set(uiFiles)

smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/ProjectPanelConfiguration.json"
  TYPE "_json"
  HEADER_OUTPUT rpJsonHeader)

set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTOUIC 1)

paraview_plugin_add_dock_window(
  CLASS_NAME pqSMTKProjectPanel
  DOCK_AREA Left
  INTERFACES dock_interfaces
  SOURCES dock_sources)

paraview_plugin_add_auto_start(
  CLASS_NAME AutoStart
  INTERFACES auto_start_interfaces
  SOURCES auto_start_sources
  )

smtk_add_plugin(smtkProjectPlugin
  REGISTRAR smtk::project::plugin::Registrar
  REGISTRAR_HEADER smtk/project/plugin/Registrar.h
  MANAGERS
    smtk::common::Managers
    smtk::operation::Manager
    smtk::project::Manager
    smtk::resource::Manager
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
    SOURCES
      ${classes}
      ${uiFiles}
      ${rpJsonHeader}
      ${dock_sources}
      ${auto_start_sources}
    UI_INTERFACES
      ${auto_start_interfaces}
      ${dock_interfaces}
)

target_link_libraries(smtkProjectPlugin
  PRIVATE
    ParaView::pqApplicationComponents
    smtkCore
    smtkPQComponentsExt
    smtkPVServerExt
)

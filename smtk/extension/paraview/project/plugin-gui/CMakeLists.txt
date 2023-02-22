paraview_plugin_add_dock_window(
  CLASS_NAME pqSMTKProjectDock
  DOCK_AREA Left
  INTERFACES dock_interfaces
  SOURCES dock_sources)

paraview_plugin_add_auto_start(
  CLASS_NAME pqSMTKProjectAutoStart
  INTERFACES auto_start_interfaces
  SOURCES auto_start_sources
  )

set(sources
  ${auto_start_sources}
  ${dock_sources}
)

set(interfaces
    ${auto_start_interfaces}
    ${dock_interfaces}
  )

paraview_add_plugin(smtkPQGuiProjectPlugin
  VERSION "1.0"
  REQUIRED_PLUGINS smtkPQCoreProjectPlugin
  UI_INTERFACES ${interfaces}
  SOURCES ${sources}
)

target_link_libraries(smtkPQGuiProjectPlugin
  PRIVATE
    ParaView::pqApplicationComponents
    smtkCore
    smtkPQComponentsExt
    smtkPQProjectExt
    smtkPVServerExt
)
target_compile_definitions(smtkPQGuiProjectPlugin PRIVATE QT_NO_KEYWORDS)

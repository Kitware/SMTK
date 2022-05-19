paraview_plugin_add_dock_window(
  CLASS_NAME pqSMTKProjectPanel
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

smtk_add_plugin(smtkPQProjectPlugin
  REGISTRAR smtk::extension::paraview::project::Registrar
  MANAGERS
    smtk::common::Managers
    smtk::operation::Manager
    smtk::project::Manager
    smtk::resource::Manager
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0"
    UI_INTERFACES ${interfaces}
    SOURCES ${sources}
)

target_link_libraries(smtkPQProjectPlugin
  PRIVATE
    ParaView::pqApplicationComponents
    smtkCore
    smtkPQComponentsExt
    smtkPQProjectExt
    smtkPVServerExt
)

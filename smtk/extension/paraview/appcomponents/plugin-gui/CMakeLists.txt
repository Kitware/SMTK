set(sources)

paraview_plugin_add_auto_start(
  CLASS_NAME pqSMTKGuiAppComponentsAutoStart
  INTERFACES auto_start_interfaces
  SOURCES auto_start_sources)

paraview_plugin_add_action_group(
  CLASS_NAME pqSMTKSelectionFilterBehavior
  GROUP_NAME "ToolBar/SMTK Selection Filters"
  INTERFACES action_group_interfaces
  SOURCES action_group_sources)
paraview_plugin_add_toolbar(
  CLASS_NAME pqSMTKColorByToolBar
  INTERFACES toolbar_interfaces
  SOURCES toolbar_sources)

paraview_plugin_add_dock_window(
  CLASS_NAME pqSMTKAttributeDock
  DOCK_AREA Left
  INTERFACES attribute_dock_interfaces
  SOURCES attribute_dock_sources)
paraview_plugin_add_dock_window(
  CLASS_NAME pqSMTKResourceDock
  DOCK_AREA Left
  INTERFACES resource_dock_interfaces
  SOURCES resource_dock_sources)
paraview_plugin_add_dock_window(
  CLASS_NAME pqSMTKTaskDock
  DOCK_AREA Left
  INTERFACES task_dock_interfaces
  SOURCES task_dock_sources)
# Keep the worklet dock **after** the task dock.
# The generated source will then register them in order,
# allowing the worklet panel to connect its palette-
# widget's signal to the task-panel's slot for emplacing
# worklets.
paraview_plugin_add_dock_window(
  CLASS_NAME pqSMTKWorkletDock
  DOCK_AREA Left
  INTERFACES worklet_dock_interfaces
  SOURCES worklet_dock_sources)

set(interfaces
  ${auto_start_interfaces}
  ${action_group_interfaces}
  ${toolbar_interfaces}
  ${proxy_interfaces}
  ${attribute_dock_interfaces}
  ${resource_dock_interfaces}
  ${task_dock_interfaces}
  ${worklet_dock_interfaces}
)
list(APPEND sources
  ${auto_start_sources}
  ${action_group_sources}
  ${toolbar_sources}
  ${proxy_sources}
  ${attribute_dock_sources}
  ${resource_dock_sources}
  ${task_dock_sources}
  ${worklet_dock_sources}
)

smtk_add_plugin(smtkPQGuiComponentsPlugin
  REGISTRARS
    smtk::extension::paraview::server::Registrar
    smtk::extension::paraview::appcomponents::Registrar
  MANAGERS
    smtk::geometry::Manager
    smtk::operation::Manager
    smtk::resource::Manager
    smtk::resource::query::Manager
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0"
    REQUIRED_ON_SERVER
    REQUIRED_ON_CLIENT
    REQUIRED_PLUGINS smtkPQComponentsPlugin
    UI_INTERFACES ${interfaces}
    SOURCES ${sources}
)
target_link_libraries(smtkPQGuiComponentsPlugin
  PRIVATE
    ParaView::pqApplicationComponents
    ParaView::VTKExtensionsExtraction
    smtkPQComponentsExt
    smtkPVServerExt
    smtkCore
    smtkQtExt
    vtkSMTKMeshExt
    vtkSMTKSourceExt
    vtkSMTKOperationsExt
)

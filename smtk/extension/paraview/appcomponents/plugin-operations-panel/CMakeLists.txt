set(sources)
set(interfaces)

paraview_plugin_add_dock_window(
  CLASS_NAME pqSMTKOperationToolboxDock
  DOCK_AREA Left
  INTERFACES operation_toolbox_dock_interfaces
  SOURCES operation_toolbox_dock_sources)
paraview_plugin_add_dock_window(
  CLASS_NAME pqSMTKOperationParameterDock
  DOCK_AREA Left
  INTERFACES operation_parameter_dock_interfaces
  SOURCES operation_parameter_dock_sources)

list(APPEND interfaces
  ${operation_parameter_dock_interfaces}
  ${operation_toolbox_dock_interfaces}
)
list(APPEND sources
  ${operation_parameter_dock_sources}
  ${operation_toolbox_dock_sources}
)

paraview_add_plugin(smtkPQOperationsPanelPlugin
    VERSION "1.0"
    REQUIRED_ON_SERVER
    REQUIRED_ON_CLIENT
    REQUIRED_PLUGINS smtkPQComponentsPlugin
    UI_FILES ${ui_files}
    UI_RESOURCES ${rc_files}
    UI_INTERFACES ${interfaces}
    SOURCES ${sources}
)
target_link_libraries(smtkPQOperationsPanelPlugin
  PRIVATE
    ParaView::pqApplicationComponents
    ParaView::VTKExtensionsExtraction
    smtkPQComponentsExt
    smtkPVServerExt
    smtkCore
    smtkQtExt
    vtkSMTKSourceExt
    vtkSMTKOperationsExt
)
target_compile_definitions(smtkPQOperationsPanelPlugin PRIVATE QT_NO_KEYWORDS)

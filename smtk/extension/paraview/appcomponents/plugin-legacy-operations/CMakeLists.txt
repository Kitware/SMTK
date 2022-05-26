set(sources)
set(interfaces)

paraview_plugin_add_dock_window(
  CLASS_NAME pqSMTKOperationDock
  DOCK_AREA Left
  INTERFACES operation_dock_interfaces
  SOURCES operation_dock_sources)

list(APPEND interfaces
  ${operation_dock_interfaces}
)
list(APPEND sources
  ${operation_dock_sources}
)

paraview_add_plugin(smtkPQLegacyOperationsPlugin
    VERSION "1.0"
    REQUIRED_ON_SERVER
    REQUIRED_ON_CLIENT
    REQUIRED_PLUGINS smtkPQComponentsPlugin
    UI_INTERFACES ${interfaces}
    SOURCES ${sources}
)
target_link_libraries(smtkPQLegacyOperationsPlugin
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
target_compile_definitions(smtkPQLegacyOperationsPlugin PRIVATE QT_NO_KEYWORDS)

set(sources)

paraview_plugin_add_action_group(
  CLASS_NAME pqSMTKSelectionFilterBehavior
  GROUP_NAME "ToolBar/SMTK Selection Filters"
  INTERFACES action_group_interfaces
  SOURCES action_group_sources)
paraview_plugin_add_toolbar(
  CLASS_NAME pqSMTKColorByToolBar
  INTERFACES toolbar_interfaces
  SOURCES toolbar_sources)

# paraview_plugin_add_proxy(
#   NAME pqServerManagerModel
#   INTERFACES proxy_interfaces
#   SOURCES proxy_sources
#   PROXY_TYPE pqSMTKResourceResourceReader
#     CLASS_NAME pqSMTKResource
#     XML_GROUP sources
#     XML_NAME SMTKResourceReader
#   PROXY_TYPE pqSMTKResourceCreator
#     CLASS_NAME pqSMTKResource
#     XML_GROUP sources
#     XML_NAME SMTKResourceCreator
#   PROXY_TYPE pqSMTKResourceImporter
#     CLASS_NAME pqSMTKResource
#     XML_GROUP sources
#     XML_NAME_REGEX SMTKResourceImporter.*)

paraview_plugin_add_dock_window(
  CLASS_NAME pqSMTKAttributePanel
  DOCK_AREA Left
  INTERFACES attribute_dock_interfaces
  SOURCES attribute_dock_sources)
paraview_plugin_add_dock_window(
  CLASS_NAME pqSMTKResourcePanel
  DOCK_AREA Left
  INTERFACES resource_dock_interfaces
  SOURCES resource_dock_sources)

set(interfaces
  ${action_group_interfaces}
  ${toolbar_interfaces}
  ${proxy_interfaces}
  ${attribute_dock_interfaces}
  ${resource_dock_interfaces}
)
list(APPEND sources
  ${action_group_sources}
  ${toolbar_sources}
  ${proxy_sources}
  ${attribute_dock_sources}
  ${resource_dock_sources}
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

set(sources)

paraview_plugin_add_auto_start(
  CLASS_NAME pqSMTKGroupingAutoStart
  INTERFACES auto_start_interfaces
  SOURCES auto_start_sources)

set(interfaces
  ${auto_start_interfaces}
)
list(APPEND sources
  ${auto_start_sources}
)

# NB: The smtk_add_plugin() macro requires REGISTRARS and
#     MANAGERS, but we don't really use any in this plugin.
smtk_add_plugin(smtkPQGroupingPlugin
  REGISTRARS
    smtk::extension::paraview::appcomponents::Registrar
  MANAGERS
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0"
    REQUIRED_ON_SERVER
    REQUIRED_ON_CLIENT
    REQUIRED_PLUGINS smtkPQComponentsPlugin
    UI_INTERFACES ${interfaces}
    SOURCES ${sources}
)
target_link_libraries(smtkPQGroupingPlugin
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

set(sources)

paraview_plugin_add_proxy(
  NAME pqServerManagerModelReaders
  INTERFACES proxy_interfaces
  SOURCES proxy_sources
  PROXY_TYPE pqSMTKResourceResourceReader
    CLASS_NAME pqSMTKResource
    XML_GROUP sources
    XML_NAME SMTKResourceReader
  PROXY_TYPE pqSMTKResourceCreator
    CLASS_NAME pqSMTKResource
    XML_GROUP sources
    XML_NAME SMTKResourceCreator
  PROXY_TYPE pqSMTKResourceImporter
    CLASS_NAME pqSMTKResource
    XML_GROUP sources
    XML_NAME_REGEX SMTKResourceImporter.*)


set(interfaces
  ${proxy_interfaces}
)
list(APPEND sources
  ${proxy_sources}
)

smtk_add_plugin(smtkPQReadersPlugin
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
target_link_libraries(smtkPQReadersPlugin
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

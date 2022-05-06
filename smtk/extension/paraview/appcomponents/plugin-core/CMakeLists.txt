set(sources)

paraview_plugin_add_auto_start(
  CLASS_NAME pqSMTKAppComponentsAutoStart
  INTERFACES auto_start_interfaces
  SOURCES auto_start_sources)

paraview_plugin_add_proxy(
  NAME pqSMTKResourceRepresentation
  INTERFACES repr_proxy_interfaces
  SOURCES repr_proxy_sources
  PROXY_TYPE pqSMTKResourceRepresentation
    XML_GROUP representations
    XML_NAME SMTKResourceCompositeRepresentation)
paraview_plugin_add_proxy(
  NAME pqServerManagerModel
  INTERFACES proxy_interfaces
  SOURCES proxy_sources
  PROXY_TYPE pqSMTKResourceSource
    CLASS_NAME pqSMTKResource
    XML_GROUP sources
    XML_NAME SMTKResourceSource
  PROXY_TYPE pqSMTKResource
    XML_GROUP sources
    XML_NAME SMTKResource
  PROXY_TYPE pqSMTKWrapper
    XML_GROUP smtk
    XML_NAME SMTKWrapper
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
  ${auto_start_interfaces}
  ${repr_proxy_interfaces}
  ${proxy_interfaces}
)
list(APPEND sources
  ${auto_start_sources}
  ${repr_proxy_sources}
  ${proxy_sources}
)

smtk_add_plugin(smtkPQComponentsPlugin
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
    UI_INTERFACES ${interfaces}
    SOURCES ${sources}
    MODULES smtkPQComponentsExt
)
target_link_libraries(smtkPQComponentsPlugin
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

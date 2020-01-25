set(sources
  pqSMTKAppComponentsAutoStart.cxx
  pqSMTKAppComponentsAutoStart.h
  pqSMTKAttributePanel.cxx
  pqSMTKAttributePanel.h
  pqSMTKCallObserversOnMainThreadBehavior.cxx
  pqSMTKCallObserversOnMainThreadBehavior.h
  pqSMTKCloseResourceBehavior.cxx
  pqSMTKCloseResourceBehavior.h
  pqSMTKCloseWithActiveOperationBehavior.cxx
  pqSMTKCloseWithActiveOperationBehavior.h
  pqSMTKColorByToolBar.cxx
  pqSMTKColorByToolBar.h
  pqSMTKColorByWidget.cxx
  pqSMTKColorByWidget.h
  pqSMTKDisplayAttributeOnLoadBehavior.cxx
  pqSMTKDisplayAttributeOnLoadBehavior.h
  pqSMTKExportSimulationBehavior.cxx
  pqSMTKExportSimulationBehavior.h
  pqSMTKImportIntoResourceBehavior.cxx
  pqSMTKImportIntoResourceBehavior.h
  pqSMTKImportOperationBehavior.cxx
  pqSMTKImportOperationBehavior.h
  pqSMTKNewResourceBehavior.cxx
  pqSMTKNewResourceBehavior.h
  pqSMTKOperationPanel.cxx
  pqSMTKOperationPanel.h
  pqSMTKPipelineSelectionBehavior.cxx
  pqSMTKPipelineSelectionBehavior.h
  pqSMTKRegisterImportersBehavior.cxx
  pqSMTKRegisterImportersBehavior.h
  pqSMTKSaveOnCloseResourceBehavior.cxx
  pqSMTKSaveOnCloseResourceBehavior.h
  pqSMTKSaveResourceBehavior.cxx
  pqSMTKSaveResourceBehavior.h
  pqSMTKSelectionFilterBehavior.cxx
  pqSMTKSelectionFilterBehavior.h
  pqSMTKSubtractUI.cxx
  pqSMTKSubtractUI.h

  vtkSMTKEncodeSelection.cxx
  vtkSMTKEncodeSelection.h)

paraview_plugin_add_auto_start(
  CLASS_NAME pqSMTKAppComponentsAutoStart
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
    XML_NAME_REGEX SMTKResourceImporter.*
  PROXY_TYPE pqSMTKResource
    XML_GROUP sources
    XML_NAME SMTKResource
  PROXY_TYPE pqSMTKWrapper
    XML_GROUP smtk
    XML_NAME SMTKWrapper)

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
paraview_plugin_add_dock_window(
  CLASS_NAME pqSMTKOperationPanel
  DOCK_AREA Left
  INTERFACES operation_dock_interfaces
  SOURCES operation_dock_sources)

set(ui_files
  resources/pqSMTKColorByWidget.ui
  resources/pqSMTKOperationPanel.ui
  resources/pqSMTKSelectionFilterBehavior.ui)
set(CMAKE_AUTOUIC_SEARCH_PATHS
  "${CMAKE_CURRENT_SOURCE_DIR}/resources")

set(rc_files
  resources/pqSMTKCloseResourceBehavior.qrc
  resources/pqSMTKImportOperationBehavior.qrc
  resources/pqSMTKSaveResourceBehavior.qrc
  resources/pqSMTKSelectionFilterBehavior.qrc)

set(interfaces
  ${auto_start_interfaces}
  ${action_group_interfaces}
  ${toolbar_interfaces}
  ${repr_proxy_interfaces}
  ${proxy_interfaces}
  ${attribute_dock_interfaces}
  ${resource_dock_interfaces}
  ${operation_dock_interfaces})
list(APPEND sources
  ${auto_start_sources}
  ${action_group_sources}
  ${toolbar_sources}
  ${repr_proxy_sources}
  ${proxy_sources}
  ${attribute_dock_sources}
  ${resource_dock_sources}
  ${operation_dock_sources})

smtk_add_plugin(smtkPQComponentsPlugin
  REGISTRARS
    smtk::extension::paraview::server::Registrar
    smtk::extension::paraview::appcomponents::Registrar
  MANAGERS
    smtk::operation::Manager
    smtk::resource::Manager
    smtk::geometry::Manager
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0"
    REQUIRED_ON_SERVER
    REQUIRED_ON_CLIENT
    UI_FILES ${ui_files}
    UI_RESOURCES ${rc_files}
    UI_INTERFACES ${interfaces}
    SOURCES ${sources}
    MODULES smtkPQComponentsExt)
target_link_libraries(smtkPQComponentsPlugin
  PRIVATE
    ParaView::pqApplicationComponents
    ParaView::VTKExtensionsExtraction
    smtkPQComponentsExt
    smtkPVServerExt
    smtkCore
    smtkQtExt
    vtkSMTKSourceExt
    vtkSMTKOperationsExt)

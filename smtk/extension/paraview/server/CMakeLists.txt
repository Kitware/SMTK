set(classes
  Registrar
  RespondToVTKSelection
  VTKMeshCellSelection
  VTKModelInstancePlacementSelection
  VTKSelectionResponderGroup

  smtkModelEntityPointLocator
  vtkPVModelSources
  vtkSMSMTKResourceRepresentationProxy
  vtkSMSMTKWrapperProxy
  vtkSMTKResource
  vtkSMTKResourceCreator
  vtkSMTKResourceGenerator
  vtkSMTKResourceImporter
  vtkSMTKResourceReader
  vtkSMTKResourceRepresentation
  vtkSMTKResourceSource
  vtkSMTKSettings
  vtkSMTKWrapper)

smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/RespondToVTKSelection.sbt" smtkPVServerOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/VTKMeshCellSelection.sbt" smtkPVServerOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/VTKModelInstancePlacementSelection.sbt" smtkPVServerOperationXML)

vtk_module_add_module(smtkPVServerExt
  CLASSES ${classes}
  HEADERS_SUBDIR "smtk/extension/paraview/server")
vtk_module_link(smtkPVServerExt
  PUBLIC
    nlohmann_json
    smtkPluginSupport)

paraview_add_server_manager_xmls(
  XMLS smconfig.xml)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif ()

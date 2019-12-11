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
  CLASSES ${classes})
vtk_module_link(smtkPVServerExt
  PUBLIC
    nlohmann_json
    smtkPluginSupport)

# The list of public headers for a VTK module are stored as a "module property"
# (which is just a namespaced CMake TARGET property with some extra support for
# being different at install time). Ask for the headers and replace any
# in-source absolute paths with relative paths.
_vtk_module_get_module_property(smtkPVServerExt
  PROPERTY headers
  VARIABLE vtk_headers)
string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" vtk_headers "${vtk_headers}")
smtk_public_headers(smtkPVServerExt ${vtk_headers})

paraview_add_server_manager_xmls(
  XMLS smconfig.xml)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif ()

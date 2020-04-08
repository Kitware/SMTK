set(headers
  vtkSMTKRepresentationStyleGenerator.h
  )

set(classes
  Registrar
  VTKSelectionResponderGroup

  vtkPVModelSources
  vtkSMSMTKResourceRepresentationProxy
  vtkSMSMTKWrapperProxy
  vtkSMTKRepresentationStyleGenerator
  vtkSMTKResource
  vtkSMTKResourceCreator
  vtkSMTKResourceGenerator
  vtkSMTKResourceImporter
  vtkSMTKResourceReader
  vtkSMTKResourceRepresentation
  vtkSMTKResourceSource
  vtkSMTKSettings
  vtkSMTKWrapper)

set(pvServerOps
  RespondToVTKSelection
  VTKMeshCellSelection
  VTKModelInstancePlacementSelection
)
foreach (operator ${pvServerOps})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/${operator}.sbt"
  HEADER_OUTPUT headerName)
  list(APPEND classes ${operator})
  list(APPEND headers ${headerName})
endforeach()
# since generated headers are in this directory, we can add them without a custom target

vtk_module_add_module(smtkPVServerExt
  CLASSES ${classes}
  HEADERS ${headers}
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

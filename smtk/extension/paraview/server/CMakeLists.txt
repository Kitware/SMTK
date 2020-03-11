set(classes
  Registrar
  VTKSelectionResponderGroup

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

set(pvServerOps
  RespondToVTKSelection
  VTKMeshCellSelection
  VTKModelInstancePlacementSelection
)
foreach (operator ${pvServerOps})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/${operator}.sbt"
  TARGET_OUTPUT targetName)
  list(APPEND classes ${operator})
  list(APPEND pvServerDependencies ${targetName})
endforeach()

vtk_module_add_module(smtkPVServerExt
  CLASSES ${classes}
  HEADERS_SUBDIR "smtk/extension/paraview/server")
add_dependencies(smtkPVServerExt ${pvServerDependencies})

vtk_module_link(smtkPVServerExt
  PUBLIC
    nlohmann_json
    smtkPluginSupport)

paraview_add_server_manager_xmls(
  XMLS smconfig.xml)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif ()

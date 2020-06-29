set(headers)
set(classes
  Registrar
)

set(pvMeshOps
  VTKMeshCellSelection
)

foreach (operator ${pvMeshOps})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/${operator}.sbt"
  HEADER_OUTPUT headerName)
  list(APPEND classes ${operator})
  list(APPEND headers ${headerName})
endforeach()
# since generated headers are in this directory, we can add them without a custom target

vtk_module_add_module(smtkPVMeshExt
  CLASSES ${classes}
  HEADERS ${headers}
  HEADERS_SUBDIR "smtk/extension/paraview/mesh")

vtk_module_link(smtkPVMeshExt
  PUBLIC
    nlohmann_json
    smtkPluginSupport
    vtkSMTKMeshExt
)

# if (SMTK_ENABLE_TESTING)
#   add_subdirectory(testing)
# endif ()

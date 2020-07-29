set(headers)
set(classes
  Registrar
)

set(pvModelOps
  VTKModelInstancePlacementSelection
)

foreach (operator ${pvModelOps})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/${operator}.sbt"
  HEADER_OUTPUT headerName)
  list(APPEND classes ${operator})
  list(APPEND headers ${headerName})
endforeach()
# since generated headers are in this directory, we can add them without a custom target

vtk_module_add_module(smtkPVModelExt
  CLASSES ${classes}
  HEADERS ${headers}
  HEADERS_SUBDIR "smtk/extension/paraview/model")

vtk_module_link(smtkPVModelExt
  PUBLIC
    vtkSMTKGeometryExt
    nlohmann_json
)

# if (SMTK_ENABLE_TESTING)
#   add_subdirectory(testing)
# endif ()

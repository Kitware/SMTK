set(headers)
set(classes
  Registrar
)

vtk_module_add_module(smtkPVMarkupExt
  CLASSES ${classes}
  HEADERS ${headers}
  HEADERS_SUBDIR "smtk/extension/paraview/markup")

vtk_module_link(smtkPVMarkupExt
  PUBLIC
    nlohmann_json
    vtkSMTKMarkupExt
)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif ()

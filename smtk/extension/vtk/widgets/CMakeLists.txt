set(classes
  vtkConeRepresentation
  vtkConeWidget
  vtkSBFunctionParser
  vtkSMTKArcRepresentation)

vtk_module_add_module(vtkSMTKWidgetsExt
  CLASSES ${classes}
  HEADERS_SUBDIR "smtk/extension/vtk/widgets")

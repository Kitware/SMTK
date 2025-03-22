set(classes
  vtkConeRepresentation
  vtkConeWidget
  vtkDiskRepresentation
  vtkDiskWidget
  vtkSBFunctionParser
  vtkSMTKArcRepresentation)

vtk_module_add_module(vtkSMTKWidgetsExt
  CLASSES ${classes}
  HEADERS_SUBDIR "smtk/extension/vtk/widgets")

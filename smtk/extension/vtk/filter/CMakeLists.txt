set(classes
  vtkCleanPolylines
  vtkDEMToMesh
  vtkExtractImageBlock
  vtkExtractLine
  vtkImageDual
  vtkImageSpacingFlip
  vtkLIDARElevationFilter
  vtkPolyDataStatsFilter)
set(private_headers)

vtk_module_add_module(vtkSMTKFilterExt
  CLASSES ${classes}
  PRIVATE_HEADERS ${private_headers}
  HEADERS_SUBDIR "smtk/extension/vtk/filter")

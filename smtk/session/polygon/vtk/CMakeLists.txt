set(classes
  vtkPolygonArcOperation
  vtkPolygonArcProvider
  vtkPolygonContourOperation)

vtk_module_add_module(vtkPolygonOperationsExt
  CLASSES ${classes}
  HEADERS_SUBDIR "smtk/session/polygon/vtk")
# Link explicitly to vtkSMTKOperationsExt, which can't be set in module.cmake
# because this is invoked by separate vtk_smtk_process_modules in polygon
# session
vtk_module_link(vtkPolygonOperationsExt
  PUBLIC
    smtkCore)

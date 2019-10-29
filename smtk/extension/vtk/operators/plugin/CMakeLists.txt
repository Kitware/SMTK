paraview_add_plugin(smtkVTKOperationsPlugin
  VERSION 1.0)
target_link_libraries(smtkVTKOperationsPlugin
  PRIVATE
    vtkSMTKOperationsExt)

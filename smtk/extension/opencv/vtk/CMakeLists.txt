set(classes
  vtkSurfaceExtractorOperation)

vtk_module_add_module(vtkSMTKOpenCVExt
  CLASSES ${classes}
  HEADERS_SUBDIR "smtk/extension/opencv/vtk")
target_link_libraries(${vtk-module}
  LINK_PUBLIC
    smtkCore
  LINK_PRIVATE
  )

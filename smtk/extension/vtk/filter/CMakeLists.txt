option(SMTK_ENABLE_OPENCV "Enable OpenCV" OFF)

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

if(SMTK_ENABLE_OPENCV)
  find_package(OpenCV REQUIRED)

  list(APPEND classes
    vtkGrabCutFilter
    vtkImageClassFilter
    vtkWatershedFilter)
  list(APPEND private_headers
    vtkOpenCVHelper.h)
endif()

vtk_module_add_module(vtkSMTKFilterExt
  CLASSES ${classes}
  PRIVATE_HEADERS ${private_headers}
  HEADERS_SUBDIR "smtk/extension/vtk/filter")

if(SMTK_ENABLE_OPENCV)
  vtk_module_link(vtkSMTKFilterExt
    PRIVATE
      VTK::IOImage
      # TODO: Use imported targets for OpenCV rather than a variable.
      ${OpenCV_LIBS})
endif()

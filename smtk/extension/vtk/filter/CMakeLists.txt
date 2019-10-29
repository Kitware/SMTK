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
  PRIVATE_HEADERS ${private_headers})

if(SMTK_ENABLE_OPENCV)
  vtk_module_link(vtkSMTKFilterExt
    PRIVATE
      VTK::IOImage
      # TODO: Use imported targets for OpenCV rather than a variable.
      ${OpenCV_LIBS})
endif()

# The list of public headers for a VTK module are stored as a "module property"
# (which is just a namespaced CMake TARGET property with some extra support for
# being different at install time). Ask for the headers and replace any
# in-source absolute paths with relative paths.
_vtk_module_get_module_property(vtkSMTKFilterExt
  PROPERTY headers
  VARIABLE vtk_headers)
string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" vtk_headers "${vtk_headers}")
smtk_public_headers(vtkSMTKFilterExt ${vtk_headers})

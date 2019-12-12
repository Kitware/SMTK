set(classes
  PointCloudFromVTKAuxiliaryGeometry
  SourceFromAttribute
  SourceFromMesh
  SourceFromModel
  StructuredGridFromVTKAuxiliaryGeometry
  vtkAttributeMultiBlockSource
  vtkAuxiliaryGeometryExtension
  vtkCmbLayeredConeSource
  vtkConeFrustum
  vtkImplicitConeFrustum
  vtkMeshMultiBlockSource
  vtkModelAuxiliaryGeometry
  vtkModelMultiBlockSource
  vtkModelView
  vtkResourceMultiBlockSource)
set(templates
  vtkModelAuxiliaryGeometry.txx)
set(headers
  SourceGenerator.h
  vtkTracksAllInstances.h)

vtk_module_add_module(vtkSMTKSourceExt
  CLASSES ${classes}
  TEMPLATES ${templates}
  HEADERS ${headers})

vtk_module_link(vtkSMTKSourceExt
  PUBLIC
    smtkCore
    smtkIOVTK
  PRIVATE
    # TODO: Use Boost:: imported targets and vtk_module_find_package.
    ${Boost_LIBRARIES}
)

# The list of public headers for a VTK module are stored as a "module property"
# (which is just a namespaced CMake TARGET property with some extra support for
# being different at install time). Ask for the headers and replace any
# in-source absolute paths with relative paths.
_vtk_module_get_module_property(vtkSMTKSourceExt
  PROPERTY headers
  VARIABLE vtk_headers)
string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" vtk_headers "${vtk_headers}")
smtk_public_headers(vtkSMTKSourceExt ${vtk_headers})

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

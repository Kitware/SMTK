set(classes
  Geometry
  PointCloudFromVTKAuxiliaryGeometry
  Registrar
  SourceFromAttribute
  SourceFromMesh
  SourceFromModel
  StructuredGridFromVTKAuxiliaryGeometry
  vtkAttributeMultiBlockSource
  vtkAuxiliaryGeometryExtension
  vtkCmbLayeredConeSource
  vtkConeFrustum
  vtkImplicitConeFrustum
  vtkModelAuxiliaryGeometry
  vtkModelMultiBlockSource
  vtkModelView
  vtkResourceMultiBlockSource)
set(templates
  vtkModelAuxiliaryGeometry.txx)
set(headers
  Backend.h
  SourceGenerator.h
  vtkTracksAllInstances.h)

vtk_module_add_module(vtkSMTKSourceExt
  CLASSES ${classes}
  TEMPLATES ${templates}
  HEADERS ${headers}
  HEADERS_SUBDIR "smtk/extension/vtk/source")

vtk_module_link(vtkSMTKSourceExt
  PUBLIC
    smtkCore
    smtkIOVTK
  PRIVATE
    # TODO: Use Boost:: imported targets and vtk_module_find_package.
    ${Boost_LIBRARIES}
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

set(classes
  BoundingBox
  ClosestPoint
  DistanceTo
  Geometry
  Registrar
)
set(headers
  Backend.h
)

vtk_module_add_module(vtkSMTKGeometryExt
  CLASSES ${classes}
  TEMPLATES ${templates}
  HEADERS ${headers}
  HEADERS_SUBDIR "smtk/extension/vtk/geometry")

vtk_module_link(vtkSMTKGeometryExt
  PUBLIC
    smtkCore
  PRIVATE
    # TODO: Use Boost:: imported targets and vtk_module_find_package.
    ${Boost_LIBRARIES}
)

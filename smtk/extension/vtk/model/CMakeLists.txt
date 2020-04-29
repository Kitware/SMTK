set(classes
  vtkAuxiliaryGeometryExtension
  vtkModelAuxiliaryGeometry
)
set(templates
  vtkModelAuxiliaryGeometry.txx
)

vtk_module_add_module(vtkSMTKModelExt
  CLASSES ${classes}
  TEMPLATES ${templates}
  HEADERS_SUBDIR "smtk/extension/vtk/model")

vtk_module_link(vtkSMTKModelExt
  PUBLIC
    smtkCore
    smtkIOVTK
  PRIVATE
    # TODO: Use Boost:: imported targets and vtk_module_find_package.
    ${Boost_LIBRARIES}
)

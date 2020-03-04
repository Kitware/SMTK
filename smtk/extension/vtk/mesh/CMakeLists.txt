set(classes
  Geometry
  Registrar)

vtk_module_add_module(vtkSMTKMeshExt
  CLASSES ${classes}
  HEADERS_SUBDIR "smtk/extension/vtk/mesh")

vtk_module_link(vtkSMTKMeshExt
  PUBLIC
    smtkCore
    smtkIOVTK
    vtkSMTKSourceExt
)

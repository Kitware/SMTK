set(classes
  Geometry
  Registrar)

set(headers
  RegisterVTKBackend.h)

vtk_module_add_module(vtkSMTKMeshExt
  CLASSES ${classes}
  HEADERS ${headers}
  HEADERS_SUBDIR "smtk/extension/vtk/mesh")

vtk_module_link(vtkSMTKMeshExt
  PUBLIC
    smtkCore
    smtkIOVTK
    vtkSMTKSourceExt
)

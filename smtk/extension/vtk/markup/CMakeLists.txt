set(classes
  Geometry
  Registrar
)

set(headers
  RegisterVTKBackend.h
)

vtk_module_add_module(vtkSMTKMarkupExt
  CLASSES ${classes}
  HEADERS ${headers}
  HEADERS_SUBDIR "smtk/extension/vtk/markup"
)

vtk_module_link(vtkSMTKMarkupExt
  PUBLIC
    smtkCore
    smtkMarkup
    smtkIOVTK
    vtkSMTKSourceExt
    vtkSMTKGeometryExt
    VTK::FiltersSources
)

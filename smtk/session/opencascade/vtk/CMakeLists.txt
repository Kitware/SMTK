set(classes
  Geometry
  Registrar
)
vtk_module_add_module(vtkOpencascadeGeometryExt
  CLASSES ${classes}
  HEADERS_SUBDIR "smtk/session/opencascade/vtk"
)
vtk_module_link(vtkOpencascadeGeometryExt
  PUBLIC
    smtkCore
    smtkOpencascadeSession
    TKGeomBase
    TKBRep
    TKMesh
    TKernel
)

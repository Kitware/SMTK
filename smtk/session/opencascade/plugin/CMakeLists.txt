
smtk_add_plugin(smtkOpencascadeSessionPlugin
  REGISTRARS
    smtk::session::opencascade::Registrar
    smtk::session::opencascade::vtk::Registrar
  MANAGERS
    smtk::operation::Manager
    smtk::resource::Manager
    smtk::geometry::Manager
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
)

target_link_libraries(smtkOpencascadeSessionPlugin
  PRIVATE
    smtkOpencascadeSession
    vtkOpencascadeGeometryExt
    vtkSMTKGeometryExt
    smtkCore
)

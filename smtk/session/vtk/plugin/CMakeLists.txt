smtk_add_plugin(smtkVTKSessionPlugin
  REGISTRAR smtk::session::vtk::Registrar
  MANAGERS smtk::operation::Manager smtk::resource::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)
target_link_libraries(smtkVTKSessionPlugin
  PRIVATE
    smtkVTKSession)

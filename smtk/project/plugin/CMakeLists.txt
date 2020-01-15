smtk_add_plugin(smtkProjectPlugin
  REGISTRAR smtk::project::Registrar
  REGISTRAR_HEADER smtk/project/Registrar.h
  MANAGERS  smtk::project::Manager smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)

smtk_add_plugin(smtkResourcePlugin
  REGISTRAR smtk::resource::Registrar
  MANAGERS  smtk::common::Managers
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)

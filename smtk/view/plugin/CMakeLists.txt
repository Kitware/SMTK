smtk_add_plugin(smtkViewPlugin
  REGISTRAR smtk::view::Registrar
  MANAGERS  smtk::common::Managers
            smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)

smtk_add_plugin(smtkTaskPlugin
  REGISTRAR smtk::task::Registrar
  MANAGERS  smtk::task::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
)

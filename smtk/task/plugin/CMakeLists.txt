smtk_add_plugin(smtkTaskPlugin
  REGISTRAR smtk::task::Registrar
  MANAGERS
    smtk::operation::Manager
    smtk::resource::Manager
    smtk::task::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
)

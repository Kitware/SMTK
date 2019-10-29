smtk_add_plugin(smtkOperationPlugin
  REGISTRAR smtk::operation::Registrar
  MANAGERS  smtk::operation::Manager
            smtk::resource::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)

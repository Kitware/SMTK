smtk_add_plugin(smtkModelPlugin
  REGISTRAR smtk::model::Registrar
  MANAGERS  smtk::operation::Manager
            smtk::resource::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)

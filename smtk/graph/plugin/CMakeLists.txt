smtk_add_plugin(smtkGraphPlugin
  REGISTRAR smtk::graph::Registrar
  MANAGERS  smtk::operation::Manager
            smtk::resource::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)

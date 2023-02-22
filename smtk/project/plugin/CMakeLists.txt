smtk_add_plugin(smtkProjectPlugin
  REGISTRAR smtk::project::Registrar
  MANAGERS  smtk::project::Manager
            smtk::common::Managers
            smtk::operation::Manager
            smtk::resource::Manager
            smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)

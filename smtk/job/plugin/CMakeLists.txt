smtk_add_plugin(smtkJobPlugin
  REGISTRARS smtk::job::Registrar
  MANAGERS   smtk::common::Managers
             smtk::resource::Manager
             smtk::operation::Manager
             smtk::job::Manager
             smtk::task::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)

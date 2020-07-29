smtk_add_plugin(smtkAttributePlugin
  REGISTRAR smtk::attribute::Registrar
  MANAGERS  smtk::common::Managers
            smtk::operation::Manager
            smtk::resource::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)

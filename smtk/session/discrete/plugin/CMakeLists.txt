smtk_add_plugin(smtkDiscreteSessionPlugin
  REGISTRAR smtk::session::discrete::Registrar
  MANAGERS smtk::operation::Manager smtk::resource::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)
target_link_libraries(smtkDiscreteSessionPlugin
  PRIVATE
    smtkCore
    smtkDiscreteSession)

smtk_add_plugin(smtkMatplotlibPlugin
  REGISTRAR smtk::extension::matplotlib::Registrar
  MANAGERS smtk::operation::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)
target_link_libraries(smtkMatplotlibPlugin
  PRIVATE
    smtkMatplotlibExt)

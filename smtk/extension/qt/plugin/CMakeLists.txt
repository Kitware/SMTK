smtk_add_plugin(smtkQtPlugin
  REGISTRAR smtk::extension::qtViewRegistrar
  REGISTRAR_HEADER smtk/extension/qt/qtViewRegistrar.h
  MANAGERS
    smtk::common::Managers
    smtk::extension::qtManager
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)

target_link_libraries(smtkQtPlugin
  PRIVATE
    smtkQtExt)

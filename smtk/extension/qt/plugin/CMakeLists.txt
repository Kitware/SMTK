smtk_add_plugin(smtkQtPlugin
  REGISTRAR smtk::extension::qtViewRegistrar
  REGISTRAR_HEADER smtk/extension/qt/qtViewRegistrar.h
  MANAGERS
    # Order is significant. DO NOT REORDER.
    # Operations must be registered before common::Managers
    smtk::operation::Manager
    smtk::common::Managers
    smtk::extension::qtManager
    smtk::job::Manager
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)

target_link_libraries(smtkQtPlugin
  PRIVATE
    smtkQtExt)

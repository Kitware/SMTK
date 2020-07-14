smtk_add_plugin(smtkGeometryPlugin
  REGISTRAR smtk::geometry::Registrar
  MANAGERS  smtk::common::Managers
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)

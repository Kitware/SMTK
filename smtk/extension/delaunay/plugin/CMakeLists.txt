smtk_add_plugin(smtkDelaunayPlugin
  REGISTRAR smtk::extension::delaunay::Registrar
  MANAGERS smtk::operation::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0")
target_link_libraries(smtkDelaunayPlugin
  PRIVATE
    smtkDelaunayExt)

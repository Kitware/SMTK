smtk_add_plugin(smtkMeshSessionPlugin
  REGISTRAR smtk::session::mesh::Registrar
  MANAGERS smtk::operation::Manager smtk::resource::Manager smtk::geometry::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)
target_link_libraries(smtkMeshSessionPlugin
  PRIVATE
    smtkMeshSession)

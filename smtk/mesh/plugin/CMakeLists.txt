smtk_add_plugin(smtkMeshPlugin
  REGISTRAR smtk::mesh::Registrar
  REGISTRAR_HEADER smtk/mesh/resource/Registrar.h
  MANAGERS  smtk::operation::Manager
            smtk::resource::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0)
target_link_libraries(smtkMeshPlugin
  PRIVATE
    smtkMeshSession)

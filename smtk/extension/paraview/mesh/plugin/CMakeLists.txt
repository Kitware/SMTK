get_property(smtk_vtk_modules GLOBAL
  PROPERTY _smtk_vtk_extention_modules)

smtk_add_plugin(smtkPVMeshExtPlugin
  REGISTRAR
    smtk::extension::paraview::mesh::Registrar
  MANAGERS
    smtk::geometry::Manager
    smtk::operation::Manager
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
    REQUIRED_ON_SERVER
    REQUIRED_ON_CLIENT
    MODULES smtkPVMeshExt
)

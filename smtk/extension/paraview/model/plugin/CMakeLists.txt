get_property(smtk_vtk_modules GLOBAL
  PROPERTY _smtk_vtk_extention_modules)

smtk_add_plugin(smtkPVModelExtPlugin
  REGISTRAR
    smtk::extension::paraview::model::Registrar
  MANAGERS
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
    REQUIRED_ON_SERVER
    REQUIRED_ON_CLIENT
    MODULES smtkPVModelExt
)

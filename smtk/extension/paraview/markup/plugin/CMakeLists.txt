get_property(smtk_vtk_modules GLOBAL
  PROPERTY _smtk_vtk_extention_modules)

smtk_add_plugin(smtkPVMarkupExtPlugin
  REGISTRARS
    smtk::extension::paraview::markup::Registrar
  MANAGERS
    smtk::geometry::Manager
    smtk::operation::Manager
    smtk::resource::query::Manager
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
    REQUIRED_ON_SERVER
    REQUIRED_ON_CLIENT
    MODULES smtkPVMarkupExt
)

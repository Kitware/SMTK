get_property(smtk_vtk_modules GLOBAL
  PROPERTY _smtk_vtk_extention_modules)

paraview_add_plugin(smtkPVModelExtPlugin
  VERSION 1.0
  REQUIRED_ON_SERVER
  REQUIRED_ON_CLIENT
  MODULES smtkPVModelExt
)

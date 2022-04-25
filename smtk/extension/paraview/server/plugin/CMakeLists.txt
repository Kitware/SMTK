get_property(smtk_vtk_modules GLOBAL
  PROPERTY _smtk_vtk_extention_modules)

paraview_add_plugin(smtkPVServerExtPlugin
  VERSION 1.0
  REQUIRED_ON_SERVER
  REQUIRED_ON_CLIENT
  MODULES smtkPVServerExt
          ${smtk_vtk_modules})
target_compile_definitions(smtkPVServerExtPlugin PRIVATE QT_NO_KEYWORDS)

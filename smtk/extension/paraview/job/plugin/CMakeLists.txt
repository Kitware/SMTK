get_property(smtk_vtk_modules GLOBAL
  PROPERTY _smtk_vtk_extention_modules)

smtk_add_plugin(smtkPVJobExtPlugin
  REGISTRAR
    smtk::extension::paraview::job::Registrar
  MANAGERS
    # Order is significant. DO NOT REORDER!
    smtk::resource::Manager
    smtk::operation::Manager
    smtk::job::Manager
    smtk::view::Manager
    smtk::common::Managers
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
    REQUIRED_ON_SERVER
    REQUIRED_ON_CLIENT
    MODULES
      smtkPVJobExt
      ParaView::pqApplicationComponents
      ParaView::pqComponents
      ParaView::qttesting
)

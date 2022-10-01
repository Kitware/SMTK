smtk_add_plugin(smtkPQOperationViewsPlugin
  REGISTRAR
    smtk::extension::paraview::operators::Registrar
  MANAGERS
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0"
    REQUIRED_PLUGINS smtkPQComponentsPlugin smtkPQWidgetsPlugin
    MODULES smtkPQOperationViewsExt
)
target_link_libraries(
  smtkPQOperationViewsPlugin
  PRIVATE
    smtkPQOperationViewsExt
    smtkPQWidgetsExt
    smtkPQComponentsExt
    smtkQtExt
    ParaView::pqComponents
    ParaView::RemotingViews
)

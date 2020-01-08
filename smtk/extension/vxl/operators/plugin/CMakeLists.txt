# add a Registrar so smtkTerrainExtractionView can be created
# by the view::Manager
smtk_add_plugin(smtkVXLOperationViewsPlugin
  REGISTRARS smtk::extension::vxl::operators::Registrar
  MANAGERS smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0"
)
target_link_libraries(smtkVXLOperationViewsPlugin
  LINK_PUBLIC
    smtkVXLOperationViewsExt
)

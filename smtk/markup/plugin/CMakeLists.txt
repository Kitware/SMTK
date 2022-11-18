smtk_add_plugin(smtkMarkupPlugin
  REGISTRAR smtk::markup::Registrar
  MANAGERS  smtk::common::Managers
            smtk::operation::Manager
            smtk::resource::Manager
            smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
)
target_link_libraries(smtkMarkupPlugin
  PRIVATE
    smtkCore
    smtkMarkup
)

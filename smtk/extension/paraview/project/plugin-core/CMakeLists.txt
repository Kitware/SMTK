set(sources)

set(interfaces)

smtk_add_plugin(smtkPQCoreProjectPlugin
  REGISTRAR smtk::extension::paraview::project::Registrar
  MANAGERS
    smtk::common::Managers
    smtk::operation::Manager
    smtk::project::Manager
    smtk::resource::Manager
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0"
    REQUIRED_PLUGINS smtkProjectPlugin
    UI_INTERFACES ${interfaces}
    SOURCES ${sources}
)

target_link_libraries(smtkPQCoreProjectPlugin
  PRIVATE
    ParaView::pqApplicationComponents
    smtkCore
    smtkPQComponentsExt
    smtkPQProjectExt
    smtkPVServerExt
)

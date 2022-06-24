set(sources)

set(interfaces)

smtk_add_plugin(smtkProjectPlugin
  REGISTRAR smtk::extension::paraview::project::Registrar
  MANAGERS
    smtk::common::Managers
    smtk::operation::Manager
    smtk::project::Manager
    smtk::resource::Manager
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0"
    UI_INTERFACES ${interfaces}
    SOURCES ${sources}
)

target_link_libraries(smtkProjectPlugin
  PRIVATE
    ParaView::pqApplicationComponents
    smtkCore
    smtkPQComponentsExt
    smtkPQProjectExt
    smtkPVServerExt
)

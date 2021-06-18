if (SMTK_ENABLE_PROJECT_UI)
  set(uiClasses
    AutoStart
    pqSMTKProjectBrowser
    pqSMTKProjectMenu
    pqSMTKProjectPanel
  )

  set(uiFiles)
  foreach(class ${uiClasses})
    list(APPEND uiFiles ${class}.cxx)
  endforeach()

  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/ProjectPanelConfiguration.json"
    TYPE "_json"
    HEADER_OUTPUT rpJsonHeader)

  set(CMAKE_AUTOMOC 1)
  set(CMAKE_AUTOUIC 1)

  paraview_plugin_add_dock_window(
    CLASS_NAME pqSMTKProjectPanel
    DOCK_AREA Left
    INTERFACES dock_interfaces
    SOURCES dock_sources)

  paraview_plugin_add_auto_start(
    CLASS_NAME AutoStart
    INTERFACES auto_start_interfaces
    SOURCES auto_start_sources
    )

  set(plugin_ui_srcs
    ${uiFiles}
    ${rpJsonHeader}
    ${dock_sources}
    ${auto_start_sources}
  )

  set(plugin_ui_interfaces
      ${auto_start_interfaces}
      ${dock_interfaces}
    )
endif ()

smtk_add_plugin(smtkProjectPlugin
  REGISTRAR smtk::project::plugin::Registrar
  MANAGERS
    smtk::common::Managers
    smtk::operation::Manager
    smtk::project::Manager
    smtk::resource::Manager
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
    SOURCES
      Registrar.cxx
      ${ui_classes}
      ${plugin_ui_srcs}
    UI_INTERFACES
      ${plugin_ui_interfaces}
)

if (SMTK_ENABLE_PROJECT_UI)
  target_compile_definitions(smtkProjectPlugin PRIVATE ENABLE_PROJECT_UI)
endif ()

target_link_libraries(smtkProjectPlugin
  PRIVATE
    ParaView::pqApplicationComponents
    smtkCore
    smtkPQComponentsExt
    smtkPVServerExt
)

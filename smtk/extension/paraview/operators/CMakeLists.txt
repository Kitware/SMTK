set(sources
  Registrar.cxx
  Registrar.h
  smtkAssignColorsView.cxx
  smtkAssignColorsView.h)

set(ui_files
  smtkAssignColorsParameters.ui)

# for Qt methods
set(CMAKE_AUTOMOC 1)

smtk_add_plugin(smtkPQOperationViewsPlugin
  REGISTRAR
    smtk::extension::paraview::operators::Registrar
  MANAGERS
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0"
    UI_FILES ${ui_files}
    SOURCES ${sources})
target_link_libraries(smtkPQOperationViewsPlugin
  PRIVATE
    smtkQtExt
    VTK::jsoncpp)

smtk_export_header(smtkPQOperationViewsPlugin Exports.h)

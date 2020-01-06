set(sources
  Registrar.cxx
  Registrar.h)

set(ui_files
  smtkAssignColorsParameters.ui)

set(views
  smtkAssignColorsView)
set(interfaces)
foreach (view IN LISTS views)
  # smtk_plugin_add_ui_view(
  #   CLASS_NAME "${view}"
  #   VIEW_NAME "${view}"
  #   INTERFACES view_interfaces
  #   SOURCES view_sources)
  # list(APPEND interfaces
  #   ${view_interfaces})
  list(APPEND sources
    "${view}.cxx"
    "${view}.h"
    ${view_sources})
endforeach()

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
    # UI_INTERFACES ${interfaces}
    SOURCES ${sources})
target_link_libraries(smtkPQOperationViewsPlugin
  PRIVATE
    smtkQtExt
    VTK::jsoncpp)

smtk_export_header(smtkPQOperationViewsPlugin Exports.h)

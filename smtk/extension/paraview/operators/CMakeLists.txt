set(sources)

set(ui_files
  smtkAssignColorsParameters.ui)

set(views
  smtkAssignColorsView)
set(interfaces)
foreach (view IN LISTS views)
  smtk_plugin_add_ui_view(
    CLASS_NAME "${view}"
    VIEW_NAME "${view}"
    INTERFACES view_interfaces
    SOURCES view_sources)
  list(APPEND interfaces
    ${view_interfaces})
  list(APPEND sources
    "${view}.cxx"
    "${view}.h"
    ${view_sources})
endforeach()

paraview_add_plugin(smtkPQOperationViewsPlugin
  VERSION "1.0"
  UI_FILES ${ui_files}
  UI_INTERFACES ${interfaces}
  SOURCES ${sources})
target_link_libraries(smtkPQOperationViewsPlugin
  PRIVATE
    smtkQtExt
    VTK::jsoncpp)

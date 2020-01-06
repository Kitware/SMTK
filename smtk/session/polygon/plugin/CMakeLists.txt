set(sources
  qtPolygonEdgeOperationView.cxx
  qtPolygonEdgeOperationView.h
  Registrar.cxx
  Registrar.h)

# smtk_plugin_add_ui_view(
#   CLASS_NAME qtPolygonEdgeOperationView
#   VIEW_NAME smtkPolygonEdgeView
#   INTERFACES view_interfaces
#   SOURCES view_sources)
# list(APPEND interfaces
#   ${view_interfaces})
# list(APPEND sources
#   ${view_sources})

if (SMTK_ENABLE_VTK_SUPPORT)
  # smtk_plugin_add_ui_view(
  #   CLASS_NAME qtExtractContoursView
  #   VIEW_NAME smtkPolygonContourView
  #   INTERFACES view_interfaces
  #   SOURCES view_sources)
  # list(APPEND interfaces
  #   ${view_interfaces})
  list(APPEND sources
    qtExtractContoursView.cxx
    qtExtractContoursView.h
    ${view_sources})
endif ()

# for Qt methods
set(CMAKE_AUTOMOC 1)

smtk_add_plugin(smtkPolygonSessionPlugin
  REGISTRARS smtk::session::polygon::Registrar smtk::session::polygon::plugin::Registrar
  MANAGERS smtk::operation::Manager smtk::resource::Manager smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
    SERVER_MANAGER_XML smconfig.xml
    UI_INTERFACES ${interfaces}
    SOURCES ${sources}
    MODULES vtkPolygonOperationsExt)
target_link_libraries(smtkPolygonSessionPlugin
  PRIVATE
    smtkPolygonQtExt
    smtkPolygonSession
    smtkQtExt
    vtkSMTKOperationsExt)

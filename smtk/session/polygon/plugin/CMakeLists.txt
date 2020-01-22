set(sources
  qtPolygonEdgeOperationView.cxx
  qtPolygonEdgeOperationView.h
  Registrar.cxx
  Registrar.h)

if (SMTK_ENABLE_VTK_SUPPORT)
  list(APPEND sources
    qtExtractContoursView.cxx
    qtExtractContoursView.h)
endif ()

# for Qt methods
set(CMAKE_AUTOMOC 1)

smtk_add_plugin(smtkPolygonSessionPlugin
  REGISTRARS smtk::session::polygon::Registrar smtk::session::polygon::plugin::Registrar
  MANAGERS smtk::operation::Manager smtk::resource::Manager smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
    SERVER_MANAGER_XML smconfig.xml
    SOURCES ${sources}
    MODULES vtkPolygonOperationsExt)
target_link_libraries(smtkPolygonSessionPlugin
  PRIVATE
    smtkPolygonQtExt
    smtkPolygonSession
    smtkQtExt
    vtkSMTKOperationsExt)

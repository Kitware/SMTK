smtk_add_plugin(smtkPolygonSessionPlugin
  REGISTRARS smtk::session::polygon::Registrar
  MANAGERS
    smtk::operation::Manager
    smtk::resource::Manager
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
    SERVER_MANAGER_XML smconfig.xml
    MODULES vtkPolygonOperationsExt
)
target_link_libraries(smtkPolygonSessionPlugin
  PRIVATE
    smtkPolygonQtExt
    smtkPolygonSession
    smtkQtExt
    vtkSMTKOperationsExt
)

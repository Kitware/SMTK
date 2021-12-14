smtk_add_plugin(smtkVTKOperationsPlugin
  REGISTRAR
    smtk::extension::vtk::operators::Registrar
  MANAGERS
    smtk::operation::Manager
    smtk::resource::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
)
target_link_libraries(smtkVTKOperationsPlugin
  PRIVATE
    vtkSMTKOperationsExt
)

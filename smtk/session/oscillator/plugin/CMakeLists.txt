set(sources
  oscillatorAuxiliaryGeometryExtension.cxx
  oscillatorAuxiliaryGeometryExtension.h
)

smtk_add_plugin(smtkOscillatorSessionPlugin
  REGISTRAR smtk::session::oscillator::Registrar
  MANAGERS smtk::operation::Manager smtk::resource::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0"
    SOURCES ${sources}
)

target_link_libraries(smtkOscillatorSessionPlugin
  PRIVATE
    smtkOscillatorSession
    vtkSMTKGeometryExt
    vtkSMTKModelExt
    VTK::FiltersGeneral
    VTK::FiltersSources
)

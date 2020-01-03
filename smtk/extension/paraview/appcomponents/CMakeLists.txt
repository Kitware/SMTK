set(classes
  Registrar
  pqSMTKBehavior
  pqSMTKRenderResourceBehavior
  pqSMTKResource
  pqSMTKWrapper
  pqSMTKResourceBrowser
  pqSMTKResourceRepresentation
  pqSMTKResourcePanel)

set(CMAKE_AUTOMOC 1)

vtk_module_add_module(smtkPQComponentsExt
  CLASSES ${classes}
  HEADERS_SUBDIR "smtk/extension/paraview/appcomponents")
vtk_module_link(smtkPQComponentsExt
  PUBLIC
    smtkCore
    smtkQtExt)

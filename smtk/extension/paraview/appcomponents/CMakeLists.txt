set(classes
  pqSMTKBehavior
  pqSMTKRenderResourceBehavior
  pqSMTKResource
  pqSMTKWrapper)

set(CMAKE_AUTOMOC 1)

vtk_module_add_module(smtkPQComponentsExt
  CLASSES ${classes})
vtk_module_link(smtkPQComponentsExt
  PUBLIC
    smtkCore
    smtkQtExt)

# The list of public headers for a VTK module are stored as a "module property"
# (which is just a namespaced CMake TARGET property with some extra support for
# being different at install time). Ask for the headers and replace any
# in-source absolute paths with relative paths.
_vtk_module_get_module_property(smtkPQComponentsExt
  PROPERTY headers
  VARIABLE vtk_headers)
string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" vtk_headers "${vtk_headers}")
smtk_public_headers(smtkPQComponentsExt ${vtk_headers})

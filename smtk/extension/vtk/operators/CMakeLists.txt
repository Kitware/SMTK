set(classes
  ExportEdgesToVTK
  vtkSMTKOperation)

smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/ExportEdgesToVTK.sbt" smtkVTKOperationXML)

vtk_module_add_module(vtkSMTKOperationsExt
  CLASSES ${classes})
vtk_module_link(vtkSMTKOperationsExt
  PUBLIC
    smtkCore
    smtkIOVTK)

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif ()

# The list of public headers for a VTK module are stored as a "module property"
# (which is just a namespaced CMake TARGET property with some extra support for
# being different at install time). Ask for the headers and replace any
# in-source absolute paths with relative paths.
_vtk_module_get_module_property(vtkSMTKOperationsExt
  PROPERTY headers
  VARIABLE vtk_headers)
string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" vtk_headers "${vtk_headers}")
smtk_public_headers(vtkSMTKOperationsExt ${vtk_headers})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

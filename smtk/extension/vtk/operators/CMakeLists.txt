set(classes
  ExportEdgesToVTK
  vtkSMTKOperation)

smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/ExportEdgesToVTK.sbt" smtkVTKOperationXML)

vtk_module_add_module(vtkSMTKOperationsExt
  CLASSES ${classes}
  HEADERS_SUBDIR "smtk/extension/vtk/operators")
vtk_module_link(vtkSMTKOperationsExt
  PUBLIC
    smtkCore
    smtkIOVTK)

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif ()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

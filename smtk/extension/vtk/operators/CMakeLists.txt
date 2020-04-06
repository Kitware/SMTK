set(classes
  ExportEdgesToVTK
  vtkSMTKOperation)

smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/ExportEdgesToVTK.sbt"
  HEADER_OUTPUT opHeader)

vtk_module_add_module(vtkSMTKOperationsExt
  CLASSES ${classes}
  HEADERS ${opHeader}
  HEADERS_SUBDIR "smtk/extension/vtk/operators")
# add_dependencies(vtkSMTKOperationsExt ${optargetName})

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

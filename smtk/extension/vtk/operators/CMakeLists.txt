set(classes
  Registrar
  vtkSMTKOperation
)

set(vtkGeomOperations
  DataSetInfoInspector
  ExportEdgesToVTK
  MeshInspector
  ImageInspector
)

set(icons
  image_inspector
  mesh_inspector
)

foreach (operator ${vtkGeomOperations})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/${operator}.sbt"
    HEADER_OUTPUT headerName)
  list(APPEND vtkGeomOpSrcs ${operator}.cxx)
  list(APPEND vtkGeomOpHeaders ${operator}.h)
  list(APPEND _vtkGeomOpDependencies ${headerName})
endforeach()
add_custom_target(vtkGeomOpGenHeaders DEPENDS ${_vtkGeomOpDependencies})

set(vtkGeomOpIcons)
foreach(icon ${icons})
  smtk_encode_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/icons/${icon}.svg" TYPE "_svg" TARGET_OUTPUT targetName)
  list(APPEND vtkGeomOpIcons ${targetName})
endforeach()

vtk_module_add_module(vtkSMTKOperationsExt
  CLASSES ${classes}
  HEADERS ${vtkGeomOpHeaders}
  SOURCES ${vtkGeomOpSrcs}
  HEADERS_SUBDIR "smtk/extension/vtk/operators"
)
add_dependencies(vtkSMTKOperationsExt vtkGeomOpGenHeaders ${vtkGeomOpIcons})
vtk_module_link(vtkSMTKOperationsExt
  PUBLIC
    vtkSMTKGeometryExt
    smtkCore
    smtkIOVTK
    VTK::FiltersGeometry
    VTK::FiltersPoints
)

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif ()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

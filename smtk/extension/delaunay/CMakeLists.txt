set(delaunaySrcs
  Registrar.cxx
  io/ImportDelaunayMesh.cxx
  io/ExportDelaunayMesh.cxx
  )
set(delaunayHeaders
  Registrar.h
  io/ImportDelaunayMesh.h
  io/ExportDelaunayMesh.h
  )

set(delaunayOperators
  TessellateFaces
  TriangulateFaces
  )

foreach (operator ${delaunayOperators})
  list(APPEND delaunaySrcs operators/${operator}.cxx)
  list(APPEND delaunayHeaders operators/${operator}.h)
endforeach()

add_library(smtkDelaunayExt ${delaunaySrcs})
target_link_libraries(smtkDelaunayExt
  PRIVATE
    smtkCore
    DelaunayShape
    DelaunayMesh
    DelaunayMisc
    DelaunayValidation
    DelaunayDiscretization
)

smtk_export_header(smtkDelaunayExt Exports.h)
smtk_public_headers(smtkDelaunayExt ${delaunayHeaders})
smtk_install_library(smtkDelaunayExt)

foreach (operator ${delaunayOperators})
  smtk_operation_xml_new("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
  TARGET_OUTPUT targetName)
  list(APPEND delaunayDependencies ${targetName})
endforeach()
add_dependencies(smtkDelaunayExt ${delaunayDependencies})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif ()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

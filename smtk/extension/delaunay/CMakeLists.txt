set(source
  io/ImportDelaunayMesh.cxx
  io/ExportDelaunayMesh.cxx
  operators/TessellateFaces.cxx
  operators/TriangulateFaces.cxx)
set(headers
  io/ImportDelaunayMesh.h
  io/ExportDelaunayMesh.h
  operators/TessellateFaces.h
  operators/TriangulateFaces.h)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/TriangulateFaces.sbt" delaunayOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/TessellateFaces.sbt" delaunayOperatorXML)

add_library(smtkDelaunayExt ${source})
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
smtk_public_headers(${headers})
smtk_install_library(smtkDelaunayExt)

add_subdirectory(plugin)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_REMUS_SUPPORT)
  add_subdirectory(worker)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

set(source
  io/ImportDelaunayMesh.cxx
  io/ExportDelaunayMesh.cxx
  operators/TessellateFace.cxx
  operators/TriangulateFace.cxx)
set(headers
  io/ImportDelaunayMesh.h
  io/ExportDelaunayMesh.h
  operators/TessellateFace.h
  operators/TriangulateFace.h)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/TriangulateFace.sbt" delaunayOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/TessellateFace.sbt" delaunayOperatorXML)

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

if (SMTK_USE_PYBIND11)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_REMUS_SUPPORT)
  add_subdirectory(worker)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

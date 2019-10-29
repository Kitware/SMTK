set(source
  Registrar.cxx
  io/ImportDelaunayMesh.cxx
  io/ExportDelaunayMesh.cxx
  operators/TessellateFaces.cxx
  operators/TriangulateFaces.cxx)
set(headers
  Registrar.h
  io/ImportDelaunayMesh.h
  io/ExportDelaunayMesh.h
  operators/TessellateFaces.h
  operators/TriangulateFaces.h)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/TriangulateFaces.sbt" delaunayOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/TessellateFaces.sbt" delaunayOperationXML)

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
smtk_public_headers(smtkDelaunayExt ${headers})
smtk_install_library(smtkDelaunayExt)

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif ()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

#if (SMTK_ENABLE_REMUS_SUPPORT)
#  add_subdirectory(worker)
#endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

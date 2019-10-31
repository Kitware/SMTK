set(polygonSrcs
  Session.cxx
  SessionIOJSON.cxx
  Registrar.cxx
  Resource.cxx
  internal/ActiveFragmentTree.cxx
  internal/Fragment.cxx
  internal/Model.cxx
  internal/Neighborhood.cxx
  internal/Region.cxx
  internal/SweepEvent.cxx
  internal/Vertex.cxx
  json/jsonEdge.cxx
  json/jsonModel.cxx
  json/jsonResource.cxx
  json/jsonVertex.cxx
  operators/CleanGeometry.cxx
  operators/CreateModel.cxx
  operators/CreateVertices.cxx
  operators/CreateEdge.cxx
  operators/CreateEdgeFromPoints.cxx
  operators/CreateEdgeFromVertices.cxx
  operators/CreateFaces.cxx
  operators/CreateFacesFromEdges.cxx
  operators/Delete.cxx
  operators/DemoteVertex.cxx
  operators/ForceCreateFace.cxx
  operators/LegacyRead.cxx
  operators/Read.cxx
  operators/SplitEdge.cxx
  operators/TweakEdge.cxx
  operators/Write.cxx
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0")
  set_source_files_properties(internal/Model.cxx
    PROPERTIES
      COMPILE_DEFINITIONS "GCC_STDLIBCXX_SUPPORT_BROKEN")
endif ()

set(polygonHeaders
  Session.h
  SessionIOJSON.h
  Operation.h
  Registrar.h
  Resource.h
  internal/ActiveFragmentTree.h
  internal/Config.h
  internal/Entity.h
  internal/Fragment.h
  internal/Model.h
  internal/Model.txx
  internal/Neighborhood.h
  internal/Neighborhood.txx
  internal/Region.h
  internal/SweepEvent.h
  internal/Vertex.h
  operators/CleanGeometry.h
  operators/CreateModel.h
  operators/CreateVertices.h
  operators/CreateEdge.h
  operators/CreateEdgeFromPoints.h
  operators/CreateEdgeFromVertices.h
  operators/CreateFaces.h
  operators/CreateFacesFromEdges.h
  operators/Delete.h
  operators/DemoteVertex.h
  operators/ForceCreateFace.h
  operators/LegacyRead.h
  operators/Read.h
  operators/SplitEdge.h
  operators/TweakEdge.h
  operators/Write.h

  json/jsonEdge.h
  json/jsonModel.h
  json/jsonResource.h
  json/jsonVertex.h
)

smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CleanGeometry.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/NewCreateModel.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateModel.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateVertices.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateEdge.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateEdgeFromPoints.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateEdgeFromVertices.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateFaces.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateFacesFromEdges.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Delete.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/DemoteVertex.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ForceCreateFace.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/LegacyRead.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Read.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SplitEdge.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/TweakEdge.sbt" polygonOperationXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Write.sbt" polygonOperationXML)

set(import_vtk_depends)

# The import operator is only available if there is VTK
if(SMTK_ENABLE_VTK_SUPPORT)
  set(polygonSrcs ${polygonSrcs}
     operators/ExtractContours.cxx
     operators/Import.cxx
     )
  set(polygonHeaders ${polygonHeaders}
     operators/ExtractContours.h
     operators/Import.h
     )
  smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ExtractContours.sbt" polygonOperationXML)
  smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Import.sbt" polygonOperationXML)
  list(APPEND import_vtk_depends
    vtkSMTKReaderExt
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::FiltersGeometry
    VTK::IOParallel
    VTK::InteractionStyle
    VTK::RenderingCore
    VTK::RenderingFreeType
    VTK::RenderingOpenGL2
    VTK::vtksys
    )
endif()

add_library(smtkPolygonSession ${polygonSrcs})
if(SMTK_ENABLE_VTK_SUPPORT)
  target_compile_definitions(smtkPolygonSession PUBLIC VTK_SUPPORT)
endif()
target_link_libraries(smtkPolygonSession
  LINK_PUBLIC
    smtkCore
  LINK_PRIVATE
    ${import_vtk_depends}
    ${Boost_LIBRARIES}
  )
if (SMTK_ENABLE_VTK_SUPPORT)
  vtk_module_autoinit(
    TARGETS smtkPolygonSession
    MODULES ${import_vtk_depends})
endif ()

smtk_export_header(smtkPolygonSession Exports.h)

# Install the library and exports the library when used from a build tree
set(SMTK_EXPORT_SET SMTKPolygonModules)
smtk_install_library(smtkPolygonSession)
# Install the headers
smtk_public_headers(smtkPolygonSession ${polygonHeaders})


install(FILES PointerDefs.h DESTINATION include/smtk/${SMTK_VERSION}/smtk/session/polygon)
target_include_directories(smtkPolygonSession
  PUBLIC
    $<BUILD_INTERFACE:${smtk_SOURCE_DIR}/smtk/extension/vtk/meshing>
)

if (SMTK_ENABLE_VTK_SUPPORT)
  set(module_files
    "${CMAKE_CURRENT_SOURCE_DIR}/vtk/vtk.module")
  vtk_module_scan(
    MODULE_FILES ${module_files}
    PROVIDES_MODULES vtk_modules
    HIDE_MODULES_FROM_CACHE ON
    WANT_BY_DEFAULT ON)
  vtk_module_build(
    MODULES ${vtk_modules}
    PACKAGE SMTKPolygonExt
    INSTALL_EXPORT SMTKPolygonModules
    CMAKE_DESTINATION ${SMTK_INSTALL_CONFIG_DIR}
    TEST_DIRECTORY_NAME "NONE"
    # Handled by SMTK macros. Used in preference to VTK's since SMTK also adds
    # unit tests for each public header.
    INSTALL_HEADERS OFF)
endif ()

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(qt)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

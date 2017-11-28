set(polygonSrcs
  Session.cxx
  SessionIOJSON.cxx
  RegisterSession.cxx
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
  operators/SplitEdge.cxx
  operators/TweakEdge.cxx
  operators/ExtractContours.cxx
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
  Operator.h
  RegisterSession.h
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
  operators/SplitEdge.h
  operators/TweakEdge.h
  operators/ExtractContours.h

  json/jsonEdge.h
  json/jsonModel.h
  json/jsonResource.h
  json/jsonVertex.h
)

smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/Session.json" polygonSessionJSON)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CleanGeometry.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/NewCreateModel.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateModel.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateVertices.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateEdge.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateEdgeFromPoints.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateEdgeFromVertices.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateFaces.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateFacesFromEdges.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Delete.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/DemoteVertex.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ForceCreateFace.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SplitEdge.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/TweakEdge.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ExtractContours.sbt" polygonOperatorXML)

set(import_vtk_depends)

# The import operator is only available if there is VTK
if(SMTK_ENABLE_VTK_SUPPORT)
  include("${VTK_USE_FILE}")
  list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

  # Don't add the VTK or other subdirectories as they use the
  # vtk_module_library() macro and vtk/module.cmake file.
  # The vtk_smtk_process_modules() macro finds all subdirectories
  # containing a module.cmake file automatically.
  include (smtkVTKModules)
  vtk_smtk_setup_module_environment("SMTK")
  vtk_smtk_process_modules()

  set(polygonSrcs ${polygonSrcs}
     operators/Import.cxx
     )
  set(polygonHeaders ${polygonHeaders}
     operators/Import.h
     )
  smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Import.sbt" polygonOperatorXML)
  list(APPEND import_vtk_depends
    vtkCommonCore
    vtkCommonDataModel
    vtkFiltersGeometry
    vtkInteractionStyle
    vtkIOParallel
    vtkRenderingCore
    vtkRenderingFreeType
    vtkRenderingOpenGL2
    vtkSMTKReaderExt
    vtksys
    )
endif()

add_library(smtkPolygonSession ${polygonSrcs})
target_link_libraries(smtkPolygonSession
  LINK_PUBLIC
    smtkCore
  LINK_PRIVATE
    ${import_vtk_depends}
    ${Boost_LIBRARIES}
  )

#vtk targets don't specify an include directory through usage-requirements, so
#we manually add them here
set(includes_from_vtk)
foreach(module ${import_vtk_depends})
  list(APPEND includes_from_vtk "${${module}_INCLUDE_DIRS}")
endforeach(module)

target_include_directories(smtkPolygonSession
  PRIVATE
  ${includes_from_vtk}
  )

smtk_export_header(smtkPolygonSession Exports.h)

# Install the library and exports the library when used from a build tree
smtk_install_library(smtkPolygonSession)
# Install the headers
smtk_public_headers(${polygonHeaders})


install(FILES PointerDefs.h DESTINATION include/smtk/${SMTK_VERSION}/smtk/bridge/polygon)
target_include_directories(smtkPolygonSession
  PUBLIC
    $<BUILD_INTERFACE:${SMTK_SOURCE_DIR}/smtk/extension/vtk/meshing>
)

add_subdirectory(environment)

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(qt)
  add_subdirectory(plugin)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_bridge_polygon
    PUBLIC
    nlohmann_json)
  add_subdirectory(testing)
  target_link_libraries(TestBuild_smtk_bridge_polygon
    PUBLIC nlohmann_json)
endif()

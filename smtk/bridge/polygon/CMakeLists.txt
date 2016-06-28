set(polygonSrcs
  Session.cxx
  SessionIOJSON.cxx
  Operator.cxx
  internal/ActiveFragmentTree.cxx
  internal/Fragment.cxx
  internal/Model.cxx
  internal/Neighborhood.cxx
  internal/Region.cxx
  internal/SweepEvent.cxx
  internal/Vertex.cxx
  operators/CreateModel.cxx
  operators/CreateVertices.cxx
  operators/CreateEdge.cxx
  operators/CreateFaces.cxx
  operators/EditEdge.cxx
  operators/ForceCreateFace.cxx
  operators/SplitEdge.cxx
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
  internal/ActiveFragmentTree.h
  internal/Fragment.h
  internal/Model.h
  internal/Neighborhood.h
  internal/Neighborhood.txx
  internal/Region.h
  internal/SweepEvent.h
  operators/CreateModel.h
  operators/CreateVertices.h
  operators/CreateEdge.h
  operators/CreateFaces.h
  operators/EditEdge.h
  operators/ForceCreateFace.h
  operators/SplitEdge.h
)

smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/Session.json" polygonSessionJSON)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateModel.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateVertices.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateEdge.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CreateFaces.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/EditEdge.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ForceCreateFace.sbt" polygonOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SplitEdge.sbt" polygonOperatorXML)

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
  set(import_vtk_depends
      vtkCommonCore
      vtkCommonDataModel
      vtkInteractionStyle
      vtkIOParallel
      vtkRenderingCore
      vtkRenderingFreeType
      vtkRendering${VTK_RENDERING_BACKEND}
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

if(SMTK_ENABLE_PYTHON_WRAPPING AND Shiboken_FOUND)

  # Extract the headers from polygon library we built to give them to shiboken
  sbk_wrap_library(smtkPolygonSession
    PACKAGE smtk
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${SMTK_SOURCE_DIR}/smtk
    LOCAL_INCLUDE_DIRECTORIES
      ${SMTK_SOURCE_DIR}/smtk/common
      ${SMTK_SOURCE_DIR}/smtk/attribute
      ${SMTK_SOURCE_DIR}/smtk/model
      ${SMTK_SOURCE_DIR}/smtk/session
      ${SMTK_SOURCE_DIR}/smtk/bridge/polygon
      ${SMTK_SOURCE_DIR}/smtk/simulation
      ${SMTK_SOURCE_DIR}/smtk/io
      ${SMTK_SOURCE_DIR}/smtk/view
      ${SMTK_SOURCE_DIR}/smtk
      ${SMTK_BINARY_DIR}/smtk
      ${CMAKE_CURRENT_BINARY_DIR}
    TYPESYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/typesystem.xml
    HEADERS ${polygonHeaders}
    DEPENDS smtkCore
  )
endif()

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(qt)
  add_subdirectory(plugin)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

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

  json/jsonEdge.h
  json/jsonModel.h
  json/jsonResource.h
  json/jsonVertex.h
)

set(polygonOperators
  CleanGeometry
  CreateModel
  CreateVertices
  CreateEdge
  CreateEdgeFromPoints
  CreateEdgeFromVertices
  CreateFaces
  CreateFacesFromEdges
  Delete
  DemoteVertex
  ForceCreateFace
  ImportPPG
  LegacyRead
  Read
  SplitEdge
  TweakEdge
  Write
  )

# construct operator inputs
foreach (operator ${polygonOperators})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
  HEADER_OUTPUT headerName)
  list(APPEND polygonSrcs operators/${operator}.cxx)
  list(APPEND polygonHeaders operators/${operator}.h)
  list(APPEND _polygonDependencies ${headerName})
endforeach()

set(import_vtk_depends)

# The import operator is only available if there is VTK
if(SMTK_ENABLE_VTK_SUPPORT)
  set(polygonOperators
    ExtractContours
    Import
    )
  foreach (operator ${polygonOperators})
    smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
    HEADER_OUTPUT headerName)
    list(APPEND polygonSrcs operators/${operator}.cxx)
    list(APPEND polygonHeaders operators/${operator}.h)
    list(APPEND _polygonDependencies ${headerName})
  endforeach()
  list(APPEND import_vtk_depends
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::FiltersGeometry
    VTK::IOParallel
    VTK::IOXML
    VTK::InteractionStyle
    VTK::RenderingCore
    VTK::RenderingFreeType
    VTK::RenderingOpenGL2
    VTK::vtksys
    )
endif()

add_custom_target(polygonSessionGenHeaders DEPENDS ${_polygonDependencies})

add_library(smtkPolygonSession ${polygonSrcs})
add_dependencies(smtkPolygonSession polygonSessionGenHeaders)

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
target_compile_definitions(smtkPolygonSession PRIVATE QT_NO_KEYWORDS)

smtk_export_header(smtkPolygonSession Exports.h)

# Install the library and exports the library when used from a build tree
set(SMTK_EXPORT_SET SMTKPolygonModules)
smtk_install_library(smtkPolygonSession)
# Install the headers
smtk_public_headers(smtkPolygonSession ${polygonHeaders})

install(FILES PointerDefs.h DESTINATION include/smtk/${SMTK_VERSION}/smtk/session/polygon)

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
    HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${SMTK_VERSION}"
    TEST_DIRECTORY_NAME "NONE")
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

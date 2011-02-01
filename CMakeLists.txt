PROJECT(SimBuilderMesh)

# trust that the compilers are a match
set(CMAKE_OVERRIDE_COMPILER_MISMATCH TRUE)

FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

SET(QT_QMAKE_EXECUTABLE ${PARAVIEW_QT_QMAKE_EXECUTABLE})
FIND_PACKAGE(Qt4)

include_directories(
  ${VTKExtensions_SOURCE_DIR} ${VTKExtensions_SOURCE_DIR}/Kitware
  ${CMBModel_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../Omicron/Src
  ${CMAKE_CURRENT_SOURCE_DIR}/../Omicron/Externals/Math/Src
  ${CMAKE_CURRENT_SOURCE_DIR}/../Omicron/Externals/Alloc/Src
  ${Omicron_BINARY_DIR}/include
)


SET(vtkMeshFiles
  vtkCmbMesh.cxx
  vtkCmbModelEdgeMesh.cxx
  vtkCmbModelEntityMesh.cxx
  vtkCmbModelFaceMesh.cxx
  vtkCmbModelVertexMesh.cxx
  TriangleInterface.cxx
)

SET_SOURCE_FILES_PROPERTIES(
  vtkCmbModelEntityMesh
  ABSTRACT
)

SET_SOURCE_FILES_PROPERTIES(
  TriangleInterface
  WRAP_EXCLUDE
)

add_library(SimBuilderLibrary SHARED ${vtkMeshFiles})
target_link_libraries(SimBuilderLibrary ConceptualModel_Plugin vtkFiltering ERDC_Plugin omicron triangle_omicron tl_alloc)

# Disable manifests for plugins on Windows so that they do not need to resolve their dependencies.
if (MSVC)
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
endif(MSVC)

#add_paraview_plugin(SimBuilderMesh_Plugin "1.0"
#  SERVER_MANAGER_XML smconfigSimBuilderMesh.xml
#  SERVER_MANAGER_SOURCES ${CMBProxyFiles}
#  SERVER_SOURCES ${vtkMeshFiles}
#)

#TARGET_LINK_LIBRARIES(SimBuilderMesh_Plugin ConceptualModel_Plugin vtkFiltering ERDC_Plugin)

#if(NOT APPLE)
#  install(TARGETS SimBuilderMesh_Plugin RUNTIME DESTINATION bin
#      LIBRARY DESTINATION bin)
#endif(NOT APPLE)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif(BUILD_TESTING)


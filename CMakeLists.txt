PROJECT(SimBuilderMesh)

# trust that the compilers are a match
set(CMAKE_OVERRIDE_COMPILER_MISMATCH TRUE)

SET(QT_QMAKE_EXECUTABLE ${PARAVIEW_QT_QMAKE_EXECUTABLE})
FIND_PACKAGE(Qt4)
include(${VXL_CMAKE_DIR}/UseVXL.cmake)

message(STATUS "DiscreteModel_SOURCE_DIR='${DiscreteModel_SOURCE_DIR}'")

include_directories(
  ${CMBModel_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../Omicron/Src
  ${CMAKE_CURRENT_SOURCE_DIR}/../Omicron/Externals/Math/Src
  ${CMAKE_CURRENT_SOURCE_DIR}/../Omicron/Externals/Alloc/Src
  ${Omicron_BINARY_DIR}/include
  ${vtkDiscreteModel_SOURCE_DIR}/Model
  ${vtkCmbDiscreteModel_INCLUDE_DIRS}
  ${vtkDiscreteModel_INCLUDE_DIRS}
  ${vtkCMBFiltering_INCLUDE_DIRS}
  ${vtkCMBGraphics_INCLUDE_DIRS}
  ${vtkCMBIO_INCLUDE_DIRS}
  ${vtkCMBMeshing_INCLUDE_DIRS}
  ${vtkCMBGeneral_INCLUDE_DIRS}
)

message("include: ${vtkDiscreteModel_INCLUDE_DIRS}")

IF(MSVC_VERSION EQUAL 1400 OR MSVC_VERSION GREATER 1400 OR MSVC10)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
ENDIF(MSVC_VERSION EQUAL 1400 OR MSVC_VERSION GREATER 1400 OR MSVC10)

SET(vtkMeshFiles
  vtkCmbMesh.cxx
  vtkCmbMeshClient.cxx
  vtkCmbMeshServer.cxx
  vtkCmbMeshGridRepresentationClient.cxx
  vtkCmbMeshGridRepresentationServer.cxx
  vtkCmbModelEdgeMesh.cxx
  vtkCmbModelEdgeMeshClient.cxx
  vtkCmbModelEdgeMeshServer.cxx
  vtkCmbModelEntityMesh.cxx
  vtkCmbModelFaceMesh.cxx
  vtkCmbModelFaceMeshClient.cxx
  vtkCmbModelFaceMeshServer.cxx
  vtkCmbModelVertexMesh.cxx
)
SET(CMBProxyFiles
  vtkCmbMeshPolyDataProvider.cxx
  vtkCmbMeshWrapper.cxx
  vtkCmbMeshGridRepresentationOperator.cxx
  vtkCmbModelEdgeMeshOperator.cxx
  vtkCmbModelFaceMeshOperator.cxx
  vtkCmbMeshToModelWriter.cxx
  vtkCmbMeshToModelReader.cxx
  vtkCmbMeshToModelReadOperator.cxx
)
SET_SOURCE_FILES_PROPERTIES(
  vtkCmbMesh
  vtkCmbModelEdgeMesh
  vtkCmbModelEntityMesh
  vtkCmbModelFaceMesh
  ABSTRACT
)
# add_library(SimBuilderLibrary SHARED ${vtkMeshFiles})
# target_link_libraries(SimBuilderLibrary vtkDiscreteModel vtkCmbDiscreteModel vtkFiltering vtkCMBVTKExtensions omicron triangle_omicron tl_alloc)

# Disable manifests for plugins on Windows so that they do not need to resolve their dependencies.
if (MSVC)
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
endif(MSVC)

add_paraview_plugin(SimBuilderMesh_Plugin "1.0"
  SERVER_MANAGER_XML smconfigSimBuilderMesh.xml
  SERVER_MANAGER_SOURCES ${CMBProxyFiles}
  SERVER_SOURCES ${vtkMeshFiles}
  CS_KITS vtkCmbDiscreteModel vtkDiscreteModel vtkFiltersCore
)

# Linking against vtkCmbDiscreteModelCS is because vtkCmbMeshWrapper::SetModelWrapper()
target_link_libraries(SimBuilderMesh_Plugin
  vtkCMBFiltering vtkCMBFilteringCS
  vtkCMBGraphics vtkCMBGraphicsCS
  vtkCMBIO vtkCMBIOCS
  vtkCMBMeshing vtkCMBMeshingCS
  vtkCMBGeneral vtkCMBGeneralCS
  vtkCmbDiscreteModel vtkCmbDiscreteModelCS
  vtkDiscreteModel vtkDiscreteModelCS
  vtkFiltersCore vtkFiltersCoreCS
  omicron
  triangle_omicron
  tl_alloc
)

if(NOT APPLE)
  install(TARGETS SimBuilderMesh_Plugin RUNTIME DESTINATION bin
      LIBRARY DESTINATION bin)
endif(NOT APPLE)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif(BUILD_TESTING)


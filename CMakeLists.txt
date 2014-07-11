project(SimBuilderMesh)

include_directories(
#   ${CMBModel_SOURCE_DIR}
#   ${vtkDiscreteModel_SOURCE_DIR}/Model
#   ${vtkCmbDiscreteModel_INCLUDE_DIRS}
#   ${vtkDiscreteModel_INCLUDE_DIRS}
  ${vtkCMBFiltering_INCLUDE_DIRS}
  ${vtkCMBGraphics_INCLUDE_DIRS}
  ${vtkCMBIO_INCLUDE_DIRS}
  ${vtkCMBMeshing_INCLUDE_DIRS}
  ${vtkCMBGeneral_INCLUDE_DIRS}
)

if(MSVC_VERSION EQUAL 1400 OR MSVC_VERSION GREATER 1400 OR MSVC10)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
endif(MSVC_VERSION EQUAL 1400 OR MSVC_VERSION GREATER 1400 OR MSVC10)

set(vtkMeshFiles
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
set(CMBProxyFiles
  vtkCmbMeshPolyDataProvider.cxx
  vtkCmbMeshWrapper.cxx
  vtkCmbMeshGridRepresentationOperator.cxx
  vtkCmbModelEdgeMeshOperator.cxx
  vtkCmbModelFaceMeshOperator.cxx
  vtkCmbMeshToModelWriter.cxx
  vtkCmbMeshToModelReader.cxx
  vtkCmbMeshToModelReadOperator.cxx
)
set_source_files_properties(
  vtkCmbMesh
  vtkCmbModelEdgeMesh
  vtkCmbModelEntityMesh
  vtkCmbModelFaceMesh
  ABSTRACT
)

# Disable manifests for plugins on Windows so that they do not need to resolve their dependencies.
if (MSVC)
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
endif(MSVC)

# we unset the pv-plugin variable because that will bring in stuff
# which we don't want brought in for client-server library initialization
unset(pv-plugin)
add_paraview_plugin(SimBuilderMesh_Plugin "1.0"
  SERVER_MANAGER_XML smconfigSimBuilderMesh.xml
  SERVER_MANAGER_SOURCES ${CMBProxyFiles}
  SERVER_SOURCES ${vtkMeshFiles}
)

# Linking against vtkCmbDiscreteModelCS is because vtkCmbMeshWrapper::SetModelWrapper()
target_link_libraries(SimBuilderMesh_Plugin
  LINK_PUBLIC
    vtkCMBFiltering
    vtkCMBGraphics
    vtkCMBIO
    vtkCMBMeshing
    vtkCMBGeneral
    vtkCmbDiscreteModel
    vtkDiscreteModel
    vtkFiltersCore
  LINK_PRIVATE
    vtkCMBFilteringCS
    vtkCMBGraphicsCS
    vtkCMBIOCS
    vtkCMBMeshingCS
    vtkCMBGeneralCS
    vtkCmbDiscreteModelCS
    vtkDiscreteModelCS
    vtkFiltersCoreCS
)

target_include_directories(SimBuilderMesh_Plugin
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  )

cmb_install_plugin(SimBuilderMesh_Plugin)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif(BUILD_TESTING)


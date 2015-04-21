project(SimBuilderMesh)

include_directories(
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
  vtkCMBMesh.cxx
  vtkCMBMeshClient.cxx
  vtkCMBMeshServer.cxx
  vtkCMBMeshGridRepresentationClient.cxx
  vtkCMBMeshGridRepresentationServer.cxx
  vtkCMBModelEdgeMesh.cxx
  vtkCMBModelEdgeMeshClient.cxx
  vtkCMBModelEdgeMeshServer.cxx
  vtkCMBModelEntityMesh.cxx
  vtkCMBModelFaceMesh.cxx
  vtkCMBModelFaceMeshClient.cxx
  vtkCMBModelFaceMeshServer.cxx
  vtkCMBModelVertexMesh.cxx
)
set(CMBProxyFiles
  vtkCMBMeshPolyDataProvider.cxx
  vtkCMBMeshWrapper.cxx
  vtkCMBMeshGridRepresentationOperator.cxx
  vtkCMBModelEdgeMeshOperator.cxx
  vtkCMBModelFaceMeshOperator.cxx
  vtkCMBMeshToModelWriter.cxx
  vtkCMBMeshToModelReader.cxx
  vtkCMBMeshToModelReadOperator.cxx
)
set_source_files_properties(
  vtkCMBMesh
  vtkCMBModelEdgeMesh
  vtkCMBModelEntityMesh
  vtkCMBModelFaceMesh
  ABSTRACT
)

# Disable manifests for plugins on Windows so that they do not need to resolve their dependencies.
if (MSVC)
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
endif(MSVC)

# we unset the pv-plugin variable because that will bring in stuff
# which we don't want brought in for client-server library initialization
unset(pv-plugin)
add_paraview_plugin(SimBuilderMeshPlugin "1.0"
  SERVER_MANAGER_XML smconfigSimBuilderMesh.xml
  SERVER_MANAGER_SOURCES ${CMBProxyFiles}
  SERVER_SOURCES ${vtkMeshFiles}
)

# Linking against vtkCmbDiscreteModelCS is because vtkCMBMeshWrapper::SetModelWrapper()
target_link_libraries(SimBuilderMeshPlugin
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

target_include_directories(SimBuilderMeshPlugin
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  )

cmb_install_plugin(SimBuilderMeshPlugin)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif(BUILD_TESTING)


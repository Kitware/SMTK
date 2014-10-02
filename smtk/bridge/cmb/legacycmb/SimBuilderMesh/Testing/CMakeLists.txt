

include_directories(${SimBuilderMesh_SOURCE_DIR})

add_executable(SimBuilderMeshEventsTest SimBuilderMeshEventsTest.cxx)
target_link_libraries(SimBuilderMeshEventsTest SimBuilderMesh_Plugin)
add_short_test(SimBuilderMeshEventsTest ${EXECUTABLE_OUTPUT_PATH}/SimBuilderMeshEventsTest
${CMB_TEST_DATA_ROOT}/test2D.cmb)

add_executable(SimBuilderMeshEventsTest2 SimBuilderMeshEventsTest2.cxx)
target_link_libraries(SimBuilderMeshEventsTest2 SimBuilderMesh_Plugin)
add_short_test(SimBuilderMeshEventsTest2 ${EXECUTABLE_OUTPUT_PATH}/SimBuilderMeshEventsTest2
${CMB_TEST_DATA_ROOT}/bay.cmb)

add_executable(SimBuilderMeshRelationTest SimBuilderMeshRelationTest.cxx)
target_link_libraries(SimBuilderMeshRelationTest SimBuilderMesh_Plugin)
add_short_test(SimBuilderMeshRelationTest ${EXECUTABLE_OUTPUT_PATH}/SimBuilderMeshRelationTest
${CMB_TEST_DATA_ROOT}/test2D.cmb)

#only build these tests when we have meshing turned on
if(BUILD_MESH_WORKERS)
  include_directories(${Omicron_BINARY_DIR}/include)

  add_executable(MeshSpeedComparison MeshSpeedComparison.cxx)
  target_link_libraries(MeshSpeedComparison
    SimBuilderMesh_Plugin
    vtkCmbDiscreteModel
    vtkCMBMeshing
    omicron
    triangle_omicron
    tl_alloc
    )
endif()

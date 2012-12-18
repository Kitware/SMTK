include_directories(
  ${SimBuilderMesh_SOURCE_DIR}
)

#only build these tests when we have meshing turned on
if(BUILD_MESHING_SUPPORT)

  include_directories(
  ${Omicron_BINARY_DIR}/include
  )

  ADD_EXECUTABLE(SimBuilderMeshEventsTest SimBuilderMeshEventsTest.cxx)
  TARGET_LINK_LIBRARIES(SimBuilderMeshEventsTest SimBuilderMesh_Plugin)
  ADD_TEST(SimBuilderMeshEventsTest ${EXECUTABLE_OUTPUT_PATH}/SimBuilderMeshEventsTest
  ${ERDC_TEST_DATA_ROOT}/test2D.cmb)

  ADD_EXECUTABLE(SimBuilderMeshEventsTest2 SimBuilderMeshEventsTest2.cxx)
  TARGET_LINK_LIBRARIES(SimBuilderMeshEventsTest2 SimBuilderMesh_Plugin)
  ADD_TEST(SimBuilderMeshEventsTest2 ${EXECUTABLE_OUTPUT_PATH}/SimBuilderMeshEventsTest2
  ${ERDC_TEST_DATA_ROOT}/bay.cmb)

  ADD_EXECUTABLE(SimBuilderMeshRelationTest SimBuilderMeshRelationTest.cxx)
  TARGET_LINK_LIBRARIES(SimBuilderMeshRelationTest SimBuilderMesh_Plugin)
  ADD_TEST(SimBuilderMeshRelationTest ${EXECUTABLE_OUTPUT_PATH}/SimBuilderMeshRelationTest
  ${ERDC_TEST_DATA_ROOT}/test2D.cmb)

  ADD_EXECUTABLE(MeshSpeedComparison MeshSpeedComparison.cxx)
  target_link_libraries(MeshSpeedComparison
  SimBuilderMesh_Plugin
  omicron
  triangle_omicron
  tl_alloc
  )
endif()

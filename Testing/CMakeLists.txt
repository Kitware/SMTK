include_directories(
  ${SimBuilderMesh_SOURCE_DIR}
)

ADD_EXECUTABLE(SimBuilderMeshEventsTest SimBuilderMeshEventsTest.cxx)
TARGET_LINK_LIBRARIES(SimBuilderMeshEventsTest SimBuilderLibrary)
ADD_TEST(SimBuilderMeshEventsTest ${EXECUTABLE_OUTPUT_PATH}/SimBuilderMeshEventsTest
  ${ERDC_TEST_DATA_ROOT}/test2D.cmb)

  ADD_EXECUTABLE(SimBuilderMeshEventsTest2 SimBuilderMeshEventsTest2.cxx)
TARGET_LINK_LIBRARIES(SimBuilderMeshEventsTest2 SimBuilderLibrary)
ADD_TEST(SimBuilderMeshEventsTest2 ${EXECUTABLE_OUTPUT_PATH}/SimBuilderMeshEventsTest2
  ${ERDC_TEST_DATA_ROOT}/bay.cmb)


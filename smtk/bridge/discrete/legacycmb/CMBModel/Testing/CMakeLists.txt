ADD_EXECUTABLE(CMBSerializationTest CMBSerializationTest.cxx)
TARGET_LINK_LIBRARIES(CMBSerializationTest vtkCmbDiscreteModel)
add_short_test(ModelBuilderSerializationTest ${EXECUTABLE_OUTPUT_PATH}/CMBSerializationTest)

ADD_EXECUTABLE(CMB2DModelTopologyTest CMB2DModelTopologyTest.cxx)
TARGET_LINK_LIBRARIES(CMB2DModelTopologyTest vtkCmbDiscreteModel)
add_short_test(ModelBuilder2DModelTopologyTest ${EXECUTABLE_OUTPUT_PATH}/CMB2DModelTopologyTest)

ADD_EXECUTABLE(CMBParserV4Test CMBParserV4Test.cxx)
TARGET_LINK_LIBRARIES(CMBParserV4Test vtkCmbDiscreteModel)
add_short_test(ModelBuilderParserV4Test
  ${EXECUTABLE_OUTPUT_PATH}/CMBParserV4Test
  ${CMB_TEST_DATA_ROOT}/CMBParserV4Test.cmb )

ADD_EXECUTABLE(CMBModelEventsTest CMBModelEventsTest.cxx)
TARGET_LINK_LIBRARIES(CMBModelEventsTest vtkCmbDiscreteModel)
add_short_test(ModelBuilderModelEventsTest
  ${EXECUTABLE_OUTPUT_PATH}/CMBModelEventsTest
  ${CMB_TEST_DATA_ROOT}/test2D.cmb ${CMB_TEST_DATA_ROOT}/smooth_surface.cmb)

ADD_EXECUTABLE(BridgeTest BridgeTest.cxx)
TARGET_LINK_LIBRARIES(BridgeTest vtkCmbDiscreteModel smtkCore)
ADD_TEST(BridgeTest
  ${EXECUTABLE_OUTPUT_PATH}/BridgeTest
  ${CMB_TEST_DATA_ROOT}/test2D.cmb ${CMB_TEST_DATA_ROOT}/smooth_surface.cmb)

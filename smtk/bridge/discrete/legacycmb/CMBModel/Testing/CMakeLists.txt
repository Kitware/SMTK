ADD_EXECUTABLE(CMBSerializationTest CMBSerializationTest.cxx)
TARGET_LINK_LIBRARIES(CMBSerializationTest vtkCmbDiscreteModel)
add_short_test(ModelBuilderSerializationTest CMBSerializationTest)

ADD_EXECUTABLE(CMB2DModelTopologyTest CMB2DModelTopologyTest.cxx)
TARGET_LINK_LIBRARIES(CMB2DModelTopologyTest vtkCmbDiscreteModel)
add_short_test(ModelBuilder2DModelTopologyTest CMB2DModelTopologyTest)

ADD_EXECUTABLE(CMBParserV4Test CMBParserV4Test.cxx)
TARGET_LINK_LIBRARIES(CMBParserV4Test vtkCmbDiscreteModel)
add_short_test(ModelBuilderParserV4Test
  CMBParserV4Test
  ${CMB_TEST_DATA_ROOT}/CMBParserV4Test.cmb )

ADD_EXECUTABLE(CMBModelEventsTest CMBModelEventsTest.cxx)
TARGET_LINK_LIBRARIES(CMBModelEventsTest vtkCmbDiscreteModel)
add_short_test(ModelBuilderModelEventsTest
  CMBModelEventsTest
  ${CMB_TEST_DATA_ROOT}/test2D.cmb ${CMB_TEST_DATA_ROOT}/smooth_surface.cmb)

ADD_EXECUTABLE(BridgeTest BridgeTest.cxx)
TARGET_LINK_LIBRARIES(BridgeTest vtkCmbDiscreteModel smtkCore)
ADD_TEST(BridgeTest
  BridgeTest
  ${CMB_TEST_DATA_ROOT}/test2D.cmb ${CMB_TEST_DATA_ROOT}/smooth_surface.cmb)

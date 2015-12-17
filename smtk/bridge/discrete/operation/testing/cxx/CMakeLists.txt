ADD_EXECUTABLE(DiscreteSerializationTest DiscreteSerializationTest.cxx)
TARGET_LINK_LIBRARIES(DiscreteSerializationTest smtkDiscreteSession vtkSMTKDiscreteModel)
add_test(NAME discreteModelSerializationTest COMMAND DiscreteSerializationTest)

ADD_EXECUTABLE(Discrete2DModelTopologyTest Discrete2DModelTopologyTest.cxx)
TARGET_LINK_LIBRARIES(Discrete2DModelTopologyTest smtkDiscreteSession vtkSMTKDiscreteModel)
add_test(NAME discreteModel2DModelTopologyTest COMMAND Discrete2DModelTopologyTest)

# ADD_EXECUTABLE(DiscreteParserV4Test DiscreteParserV4Test.cxx)
# TARGET_LINK_LIBRARIES(DiscreteParserV4Test smtkDiscreteSession vtkSMTKDiscreteModel)
# add_test(NAME discreteModelParserV4Test
#  COMMAND DiscreteParserV4Test
#  ${SMTK_DATA_DIR}/cmb/CMBParserV4Test.cmb )

# ADD_EXECUTABLE(DiscreteModelEventsTest DiscreteModelEventsTest.cxx)
# TARGET_LINK_LIBRARIES(DiscreteModelEventsTest smtkDiscreteSession)
# add_test(NAME ModelBuilderModelEventsTest
#  COMMAND DiscreteModelEventsTest
#  ${SMTK_DATA_DIR}/test2D.cmb ${SMTK_DATA_DIR}/smooth_surface.cmb)

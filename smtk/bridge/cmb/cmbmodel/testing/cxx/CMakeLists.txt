ADD_EXECUTABLE(DiscreteSerializationTest DiscreteSerializationTest.cxx)
TARGET_LINK_LIBRARIES(DiscreteSerializationTest smtkDiscreteBridge vtkSMTKDiscreteModel)
add_test(discreteModelSerializationTest ${EXECUTABLE_OUTPUT_PATH}/DiscreteSerializationTest)

ADD_EXECUTABLE(Discrete2DModelTopologyTest Discrete2DModelTopologyTest.cxx)
TARGET_LINK_LIBRARIES(Discrete2DModelTopologyTest smtkDiscreteBridge vtkSMTKDiscreteModel)
add_test(discreteModel2DModelTopologyTest ${EXECUTABLE_OUTPUT_PATH}/Discrete2DModelTopologyTest)

# ADD_EXECUTABLE(DiscreteParserV4Test DiscreteParserV4Test.cxx)
# TARGET_LINK_LIBRARIES(DiscreteParserV4Test smtkDiscreteBridge vtkSMTKDiscreteModel)
# add_test(discreteModelParserV4Test
#  ${EXECUTABLE_OUTPUT_PATH}/DiscreteParserV4Test
#  ${SMTK_DATA_DIR}/cmb/CMBParserV4Test.cmb )

# ADD_EXECUTABLE(DiscreteModelEventsTest DiscreteModelEventsTest.cxx)
# TARGET_LINK_LIBRARIES(DiscreteModelEventsTest smtkDiscreteBridge)
# add_test(ModelBuilderModelEventsTest
#  ${EXECUTABLE_OUTPUT_PATH}/DiscreteModelEventsTest
#  ${SMTK_DATA_DIR}/test2D.cmb ${SMTK_DATA_DIR}/smooth_surface.cmb)

ADD_EXECUTABLE(CMBSerializationTest CMBSerializationTest.cxx)
TARGET_LINK_LIBRARIES(CMBSerializationTest smtkCMBBridge vtkSMTKDiscreteModel)
add_test(cmbModelSerializationTest ${EXECUTABLE_OUTPUT_PATH}/CMBSerializationTest)

ADD_EXECUTABLE(CMB2DModelTopologyTest CMB2DModelTopologyTest.cxx)
TARGET_LINK_LIBRARIES(CMB2DModelTopologyTest smtkCMBBridge vtkSMTKDiscreteModel)
add_test(cmbModel2DModelTopologyTest ${EXECUTABLE_OUTPUT_PATH}/CMB2DModelTopologyTest)

# ADD_EXECUTABLE(CMBParserV4Test CMBParserV4Test.cxx)
# TARGET_LINK_LIBRARIES(CMBParserV4Test smtkCMBBridge vtkSMTKDiscreteModel)
# add_test(cmbModelParserV4Test
#  ${EXECUTABLE_OUTPUT_PATH}/CMBParserV4Test
#  ${SMTK_DATA_DIR}/cmb/CMBParserV4Test.cmb )

# ADD_EXECUTABLE(CMBModelEventsTest CMBModelEventsTest.cxx)
# TARGET_LINK_LIBRARIES(CMBModelEventsTest smtkCMBBridge)
# add_test(ModelBuilderModelEventsTest
#  ${EXECUTABLE_OUTPUT_PATH}/CMBModelEventsTest
#  ${SMTK_DATA_DIR}/test2D.cmb ${SMTK_DATA_DIR}/smooth_surface.cmb)

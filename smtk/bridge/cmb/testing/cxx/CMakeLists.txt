
ADD_EXECUTABLE(BridgeTest BridgeTest.cxx)
TARGET_LINK_LIBRARIES(BridgeTest vtkSMTKCMBModel SMTKCore)
ADD_TEST(BridgeTest
  ${EXECUTABLE_OUTPUT_PATH}/BridgeTest
  ${CMB_TEST_DATA_ROOT}/test2D.cmb ${CMB_TEST_DATA_ROOT}/smooth_surface.cmb)

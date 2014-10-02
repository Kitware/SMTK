
ADD_EXECUTABLE(BridgeTest BridgeTest.cxx)
TARGET_LINK_LIBRARIES(BridgeTest SMTKCore smtkCMBBridge)
ADD_TEST(cmbBridgeTest
  ${EXECUTABLE_OUTPUT_PATH}/BridgeTest
  ${SMTK_DATA_DIR}/cmb/test2D.cmb ${SMTK_DATA_DIR}/cmb/smooth_surface.cmb)

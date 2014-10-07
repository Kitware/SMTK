if(SMTK_DATA_DIR)
  ADD_EXECUTABLE(BridgeTest BridgeTest.cxx)
  ADD_TEST(cmbBridgeTest
    ${EXECUTABLE_OUTPUT_PATH}/BridgeTest
    ${SMTK_DATA_DIR}/cmb/test2D.cmb ${SMTK_DATA_DIR}/cmb/smooth_surface.cmb)
endif()

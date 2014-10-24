if(SMTK_DATA_DIR)
  ADD_EXECUTABLE(BridgeTest BridgeTest.cxx)
  TARGET_LINK_LIBRARIES(BridgeTest SMTKCore smtkDiscreteBridge)
  ADD_TEST(discreteBridgeTest
    ${EXECUTABLE_OUTPUT_PATH}/BridgeTest
    ${SMTK_DATA_DIR}/cmb/test2D.cmb ${SMTK_DATA_DIR}/cmb/smooth_surface.cmb)
endif()
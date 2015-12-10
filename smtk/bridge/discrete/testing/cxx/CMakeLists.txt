if(SMTK_DATA_DIR)
  ADD_EXECUTABLE(SessionTest SessionTest.cxx)
  TARGET_LINK_LIBRARIES(SessionTest smtkCore smtkDiscreteSession)
  ADD_TEST(discreteSessionTest
    SessionTest
    ${SMTK_DATA_DIR}/cmb/test2D.cmb ${SMTK_DATA_DIR}/cmb/smooth_surface.cmb)
endif()

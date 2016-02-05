if(SMTK_DATA_DIR)
  add_executable(SessionTest SessionTest.cxx)
  target_link_libraries(SessionTest smtkCore smtkDiscreteSession)
  add_test(
    NAME discreteSessionTest
    COMMAND SessionTest ${SMTK_DATA_DIR}/cmb/test2D.cmb ${SMTK_DATA_DIR}/cmb/smooth_surface.cmb
  )
endif()

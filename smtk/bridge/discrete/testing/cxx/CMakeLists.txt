if(SMTK_DATA_DIR)
  add_executable(SessionTest SessionTest.cxx)
  target_link_libraries(SessionTest smtkCore smtkDiscreteSession)
  add_test(
    NAME discreteSessionTest
    COMMAND SessionTest ${SMTK_DATA_DIR}/cmb/test2D.cmb ${SMTK_DATA_DIR}/cmb/smooth_surface.cmb
  )

  add_executable(Import2dmTest import2dm.cxx)
  target_link_libraries(Import2dmTest
    smtkCore
    smtkDiscreteSession
    vtkSMTKExt
    vtkRenderingGL2PS${VTK_RENDERING_BACKEND}
    vtkTestingRendering
  )
  add_test(
    NAME discreteImport2dmTest
    COMMAND
      $<TARGET_FILE:Import2dmTest>
      "${SMTK_DATA_DIR}/mesh/test2D.2dm"
      -V "${SMTK_DATA_DIR}/baselines/discrete/import2dm.png"
      -T "${CMAKE_CURRENT_BINARY_DIR}"
  )
endif()

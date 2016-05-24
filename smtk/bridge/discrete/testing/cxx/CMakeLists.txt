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
    vtkSMTKSourceExt
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

  add_executable(EdgeOp2dmTest edgeOp2dm.cxx)
  target_link_libraries(EdgeOp2dmTest
    smtkCore
    smtkDiscreteSession
    vtkSMTKSourceExt
    vtkRenderingGL2PS${VTK_RENDERING_BACKEND}
    vtkTestingRendering
  )
  add_test(
    NAME discreteEdgeOp2dmTest
    COMMAND
      $<TARGET_FILE:EdgeOp2dmTest>
      "${SMTK_DATA_DIR}/mesh/test2D.2dm"
      -V "${SMTK_DATA_DIR}/baselines/discrete/edgeOp2dm.png"
      -T "${CMAKE_CURRENT_BINARY_DIR}"
  )

  add_executable(smtkModelfrom2dmTest smtkModelfrom2dm.cxx)
  target_link_libraries(smtkModelfrom2dmTest
    smtkCore
    smtkDiscreteSession
    vtkSMTKSourceExt
    vtkRenderingGL2PS${VTK_RENDERING_BACKEND}
    vtkTestingRendering
    ${Boost_LIBRARIES}
  )
  target_compile_definitions(smtkModelfrom2dmTest PRIVATE "SMTK_DATA_DIR=\"${SMTK_DATA_DIR}\"")
  add_test(
    NAME discreteSmtkModelfrom2dmTest
    COMMAND
      $<TARGET_FILE:smtkModelfrom2dmTest>
      "${SMTK_DATA_DIR}/mesh/test2D.2dm"
      -V "${SMTK_DATA_DIR}/baselines/discrete/smtkModelfrom2dm.png"
      -T "${CMAKE_CURRENT_BINARY_DIR}"
  )

endif()

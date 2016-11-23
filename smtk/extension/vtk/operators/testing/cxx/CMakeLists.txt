if (SMTK_DATA_DIR)
  # test to apply BO to a model
  add_executable(BathymetryTestChesapeakeBay BathymetryTestChesapeakeBay.cxx)
  target_link_libraries(BathymetryTestChesapeakeBay
    smtkCore 
    vtkSMTKOperatorsExt 
    smtkDiscreteSession
    vtkSMTKSourceExt
    vtkRenderingGL2PSOpenGL2
    vtkTestingRendering)
  add_test(
       NAME BathymetryTestChesapeakeBay
       COMMAND BathymetryTestChesapeakeBay ${SMTK_DATA_DIR}/model/2d/cmb/ChesapeakeBayContour.cmb ${SMTK_DATA_DIR}/dem/ChesapeakeBay100x100.vti -V "${SMTK_DATA_DIR}/baseline/smtk/vtk/bathymetryTestChesapeakeBay.png"      -T "${CMAKE_CURRENT_BINARY_DIR}"
      )
  # test to apply BO to model and mesh
  add_executable(BathymetryTestSimple2dm BathymetryTestSimple2dm.cxx)
  target_link_libraries(BathymetryTestSimple2dm
    smtkCore 
    vtkSMTKOperatorsExt 
    smtkDiscreteSession
    vtkSMTKSourceExt
    vtkRenderingGL2PSOpenGL2
    vtkTestingRendering)
  add_test(
       NAME BathymetryTestSimple2dm
       COMMAND BathymetryTestSimple2dm ${SMTK_DATA_DIR}/mesh/2d/Simple.2dm ${SMTK_DATA_DIR}/mesh/2d/SimpleBathy.2dm -V "${SMTK_DATA_DIR}/baseline/smtk/vtk/bathymetrySimple2dm.png"      -T "${CMAKE_CURRENT_BINARY_DIR}"
       )
  # test to apply BO to polyon session
  add_executable(BathymetryTestNewRiversmtk BathymetryTestNewRiversmtk.cxx)
  target_link_libraries(BathymetryTestNewRiversmtk
    smtkCore
    vtkSMTKOperatorsExt
    smtkPolygonSession
    vtkSMTKSourceExt
    vtkRenderingGL2PSOpenGL2
    vtkTestingRendering
    vtkSMTKReaderExt)
  add_test(
       NAME BathymetryTestNewRiversmtk
       COMMAND BathymetryTestNewRiversmtk ${SMTK_DATA_DIR}/mesh/2d/model1meshed.smtk ${SMTK_DATA_DIR}/dem/NewRiver1M.dem -V "${SMTK_DATA_DIR}/baseline/smtk/vtk/BathymetryTestNewRiversmtk.png"      -T "${CMAKE_CURRENT_BINARY_DIR}"
       )
endif ()

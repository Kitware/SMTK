if (SMTK_DATA_DIR)
  add_executable(TestElevateMesh TestElevateMesh.cxx)
  target_link_libraries(TestElevateMesh
    smtkCore
    vtkSMTKOperatorsExt
    smtkDiscreteSession
    vtkSMTKSourceExt
    vtkRenderingGL2PSOpenGL2
    vtkTestingRendering)
  add_test(
       NAME TestElevateMesh
       COMMAND TestElevateMesh ${SMTK_DATA_DIR}/mesh/2d/Simple.2dm ${SMTK_DATA_DIR}/mesh/2d/SimpleBathy.2dm
       )

  if (SMTK_ENABLE_POLYGON_SESSION)
    # test of polygon session's delete operator with success
    add_executable(DeleteSmtkCell DeleteSmtkCell.cxx)
    target_link_libraries(DeleteSmtkCell
      smtkCore
      vtkSMTKOperatorsExt
      smtkPolygonSession
      vtkSMTKSourceExt
      vtkRenderingGL2PSOpenGL2
      vtkTestingRendering
      vtkSMTKReaderExt)
    add_test(
         NAME DeleteSmtkCell
         COMMAND DeleteSmtkCell ${SMTK_DATA_DIR}/model/2d/smtk/simple.smtk  -V "${SMTK_DATA_DIR}/baseline/smtk/vtk/DeleteSmtkCell.png"      -T "${CMAKE_CURRENT_BINARY_DIR}"
         )
    # test of polygon session's delete operator with failure
    add_executable(DeleteSmtkCell2 DeleteSmtkCell2.cxx)
    target_link_libraries(DeleteSmtkCell2
      smtkCore
      vtkSMTKOperatorsExt
      smtkPolygonSession
      vtkSMTKSourceExt)
    add_test(
         NAME DeleteSmtkCell2
         COMMAND DeleteSmtkCell2 ${SMTK_DATA_DIR}/model/2d/smtk/simple.smtk -T "${CMAKE_CURRENT_BINARY_DIR}"
         )
   endif()
endif ()

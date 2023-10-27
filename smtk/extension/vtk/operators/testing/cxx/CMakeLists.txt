if (SMTK_DATA_DIR AND SMTK_ENABLE_POLYGON_SESSION)
  # test of polygon session's delete operator with success
  add_executable(DeleteSmtkCell DeleteSmtkCell.cxx)
  target_link_libraries(DeleteSmtkCell
    smtkCore
    smtkPolygonSession
    vtkSMTKSourceExt
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::IOXML
    VTK::InteractionStyle
    VTK::RenderingCore
    VTK::TestingRendering
    )
  add_test(
    NAME DeleteSmtkCell
    COMMAND DeleteSmtkCell ${SMTK_DATA_DIR}/model/2d/smtk/simple.smtk  -V "${SMTK_DATA_DIR}/baseline/smtk/vtk/DeleteSmtkCell.png"      -T "${CMAKE_CURRENT_BINARY_DIR}"
    )
  # test of polygon session's delete operator with failure
  add_executable(DeleteSmtkCell2 DeleteSmtkCell2.cxx)
  target_link_libraries(DeleteSmtkCell2
    smtkCore
    smtkPolygonSession
    vtkSMTKSourceExt
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::IOXML
    VTK::InteractionStyle
    VTK::RenderingCore
    VTK::TestingRendering
    )
  add_test(
    NAME DeleteSmtkCell2
    COMMAND DeleteSmtkCell2 ${SMTK_DATA_DIR}/model/2d/smtk/simple.smtk -T "${CMAKE_CURRENT_BINARY_DIR}"
    )
endif ()

if (SMTK_DATA_DIR)
  # test of ExportFaceset operation (exporting geometry resource to STL file.)
  add_executable(ExportFacesetTest ExportFacesetTest.cxx)
  target_link_libraries(ExportFacesetTest
    smtkCore
    smtkMeshSession
    vtkSMTKOperationsExt
    vtkSMTKSourceExt
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::IOXML
    )
  add_test(
    NAME ExportFacesetTest
    COMMAND ExportFacesetTest -T "${CMAKE_CURRENT_BINARY_DIR}"
    )
endif ()

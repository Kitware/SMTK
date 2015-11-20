add_executable(displayModel MACOSX_BUNDLE displayModel.cxx)
target_link_libraries(displayModel
  smtkCore
  vtkSMTKExt
  vtkTestingRendering
  ${SMTK_VTK_TARGET_LIST}
)

add_executable(displayMultiBlockModel MACOSX_BUNDLE displayMultiBlockModel.cxx)
target_link_libraries(displayMultiBlockModel
  smtkCore
  vtkSMTKExt
  vtkTestingRendering
)

add_executable(displayMultiBlockMesh MACOSX_BUNDLE displayMultiBlockMesh.cxx)
target_link_libraries(displayMultiBlockMesh
  smtkCore
  vtkSMTKExt
  vtkTestingRendering
)

# Only run tests if the data directory exists
if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/ReadMe.mkd)
  add_test(
    NAME displayModel
    COMMAND
      $<TARGET_FILE:displayModel>
      "${SMTK_DATA_DIR}/smtk/simple.json"
      -V "${SMTK_DATA_DIR}/baselines/vtk/simple.png"
      -T "${CMAKE_CURRENT_BINARY_DIR}"
  )

  add_test(
    NAME displayMultiBlockModel-simple
    COMMAND
      $<TARGET_FILE:displayMultiBlockModel>
      "${SMTK_DATA_DIR}/smtk/simple.json"
      -V "${SMTK_DATA_DIR}/baselines/vtk/simple-with-colors.png"
      -T "${CMAKE_CURRENT_BINARY_DIR}"
  )

  add_test(
    NAME displayMultiBlockModel-test2D
    COMMAND
      $<TARGET_FILE:displayMultiBlockModel>
      "${SMTK_DATA_DIR}/smtk/test2D.json"
      -V "${SMTK_DATA_DIR}/baselines/vtk/test2D-with-colors.png"
      -T "${CMAKE_CURRENT_BINARY_DIR}"
  )

  add_test(
    NAME displayMultiBlockMesh-twoassm
    COMMAND
      $<TARGET_FILE:displayMultiBlockMesh>
      "${SMTK_DATA_DIR}/mesh/twoassm_out.h5m"
      -V "${SMTK_DATA_DIR}/baselines/vtk/twoassm_out.png"
      -T "${CMAKE_CURRENT_BINARY_DIR}"
  )
endif()

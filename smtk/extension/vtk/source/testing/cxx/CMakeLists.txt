add_executable(displayConeFrustum MACOSX_BUNDLE displayConeFrustum.cxx)
target_link_libraries(displayConeFrustum
  smtkCore
  vtkRenderingGL2PSOpenGL2
  vtkSMTKSourceExt
  vtkSMTKWidgetsExt
  vtkTestingRendering
)

add_executable(displayMultiBlockModel MACOSX_BUNDLE displayMultiBlockModel.cxx)
target_link_libraries(displayMultiBlockModel
  smtkCore
  vtkRenderingGL2PSOpenGL2
  vtkSMTKSourceExt
  vtkTestingRendering
)

add_executable(displayMultiBlockMesh MACOSX_BUNDLE displayMultiBlockMesh.cxx)
target_link_libraries(displayMultiBlockMesh
  smtkCore
  vtkRenderingGL2PSOpenGL2
  vtkSMTKSourceExt
  vtkTestingRendering
)

add_executable(displayModelToMesh MACOSX_BUNDLE displayModelToMesh.cxx)
target_link_libraries(displayModelToMesh
  smtkCore
  vtkRenderingGL2PSOpenGL2
  vtkSMTKSourceExt
  vtkTestingRendering
)

# Only run tests if the data directory exists
if (SMTK_DATA_DIR)
  add_test(
    NAME displayMultiBlockModel-test2D
    COMMAND
      $<TARGET_FILE:displayMultiBlockModel>
      "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json"
      -V "${SMTK_DATA_DIR}/baseline/smtk/vtk/test2D-with-colors.png"
      -T "${CMAKE_CURRENT_BINARY_DIR}"
  )

  add_test(
    NAME displayModelToMesh-simple
    COMMAND
      $<TARGET_FILE:displayModelToMesh>
      "${SMTK_DATA_DIR}/model/3d/smtk/simple.json"
      -V "${SMTK_DATA_DIR}/baseline/smtk/vtk/simple-Model2Mesh.png"
      -T "${CMAKE_CURRENT_BINARY_DIR}"
  )

  if(MOAB_HAVE_HDF5)
    add_test(
      NAME displayMultiBlockMesh-twoassm
      COMMAND
        $<TARGET_FILE:displayMultiBlockMesh>
        "${SMTK_DATA_DIR}/mesh/3d/twoassm_out.h5m"
        -V "${SMTK_DATA_DIR}/baseline/smtk/vtk/twoassm_out.png"
        -T "${CMAKE_CURRENT_BINARY_DIR}"
    )
  endif()
endif()

set(unit_tests
  unitResourceMultiBlockSource.cxx
)
set(unit_tests_which_require_data
)

smtk_unit_tests(
  LABEL "VTK"
  SOURCES ${unit_tests}
  # SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES
    smtkCore
    smtkCoreModelTesting
    vtkSMTKSourceExt
    ${extra_libs}
    ${Boost_LIBRARIES}
)

add_executable(displayConeFrustum MACOSX_BUNDLE displayConeFrustum.cxx)
target_link_libraries(displayConeFrustum
  smtkCore
  vtkSMTKSourceExt
  vtkSMTKWidgetsExt
  VTK::CommonCore
  VTK::CommonDataModel
  VTK::FiltersSources
  VTK::IOXML
  VTK::InteractionStyle
  VTK::RenderingCore
  VTK::RenderingOpenGL2
  VTK::TestingRendering
)

add_executable(displayMultiBlockModel MACOSX_BUNDLE displayMultiBlockModel.cxx)
target_link_libraries(displayMultiBlockModel
  smtkCore
  vtkSMTKSourceExt
  VTK::CommonCore
  VTK::CommonDataModel
  VTK::IOXML
  VTK::InteractionStyle
  VTK::RenderingCore
  VTK::RenderingOpenGL2
  VTK::TestingRendering
)

add_executable(displayMultiBlockMesh MACOSX_BUNDLE displayMultiBlockMesh.cxx)
target_link_libraries(displayMultiBlockMesh
  smtkCore
  vtkSMTKSourceExt
  VTK::CommonCore
  VTK::CommonDataModel
  VTK::IOXML
  VTK::InteractionStyle
  VTK::RenderingCore
  VTK::RenderingOpenGL2
  VTK::TestingRendering
)

add_executable(displayModelToMesh MACOSX_BUNDLE displayModelToMesh.cxx)
target_link_libraries(displayModelToMesh
  smtkCore
  vtkSMTKSourceExt
  VTK::CommonCore
  VTK::CommonDataModel
  VTK::IOXML
  VTK::InteractionStyle
  VTK::RenderingCore
  VTK::RenderingOpenGL2
  VTK::TestingRendering
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

  if(SMTK_ENABLE_MESH_SESSION)
    add_executable(displayAuxiliaryGeometry MACOSX_BUNDLE displayAuxiliaryGeometry.cxx)
    target_link_libraries(displayAuxiliaryGeometry
      smtkCore
      smtkMeshSession
      vtkRenderingGL2PSOpenGL2
      vtkSMTKSourceExt
      vtkTestingRendering
    )

    add_test(
      NAME displayAuxiliaryGeometry
      COMMAND
        $<TARGET_FILE:displayAuxiliaryGeometry>
        "${SMTK_DATA_DIR}/model/3d/obj/cone.obj"
        -V "${SMTK_DATA_DIR}/baseline/smtk/vtk/auxiliaryGeometry.png"
        -T "${CMAKE_CURRENT_BINARY_DIR}"
    )
  endif()

endif()

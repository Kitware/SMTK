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

set(additional_source_tests
)

add_executable(displayConeFrustum MACOSX_BUNDLE displayConeFrustum.cxx)
list(APPEND additional_source_tests displayConeFrustum)
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
list(APPEND additional_source_tests displayMultiBlockModel)
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
endif()

vtk_module_autoinit(
  TARGETS UnitTests_smtk_extension_vtk_source_testing_cxx
          ${additional_source_tests}
  MODULES VTK::CommonCore
          VTK::CommonDataModel
          VTK::FiltersSources
          VTK::InteractionStyle
          VTK::IOXML
          VTK::RenderingCore
          VTK::RenderingOpenGL2
          VTK::TestingRendering
          vtkSMTKSourceExt)

add_executable(displayModel MACOSX_BUNDLE displayModel.cxx)
target_link_libraries(displayModel SMTKCore vtkSMTK ${VTK_MODULES})
add_test(
  NAME displayModel
  COMMAND
    $<TARGET_FILE:displayModel>
    "${SMTK_SOURCE_DIR}/smtk/model/testing/cube.json"
)

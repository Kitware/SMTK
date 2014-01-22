add_executable(displayModel MACOSX_BUNDLE displayModel.cxx)
target_link_libraries(displayModel SMTKCore pvSMTK ${VTK_MODULES})
add_test(
  NAME displayModel
  COMMAND
    $<TARGET_FILE:displayModel>
    "${SMTK_SOURCE_DIR}/smtk/model/testing/cube.json"
)

add_executable(displayMultiBlockModel MACOSX_BUNDLE displayMultiBlockModel.cxx)
target_link_libraries(displayMultiBlockModel SMTKCore pvSMTK ${VTK_MODULES})
add_test(
  NAME displayMultiBlockModel
  COMMAND
    $<TARGET_FILE:displayMultiBlockModel>
    "${SMTK_SOURCE_DIR}/smtk/model/testing/cube.json"
)

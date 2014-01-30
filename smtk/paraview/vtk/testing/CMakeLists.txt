add_executable(displayModel MACOSX_BUNDLE displayModel.cxx)
target_link_libraries(displayModel SMTKCore vtkSMTK ${VTK_MODULES})

add_executable(displayMultiBlockModel MACOSX_BUNDLE displayMultiBlockModel.cxx)
target_link_libraries(displayMultiBlockModel SMTKCore vtkSMTK ${VTK_MODULES})

# Only run tests if the data directory exists
if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/ReadMe.mkd)
  add_test(
    NAME displayModel
    COMMAND
      $<TARGET_FILE:displayModel>
      "${SMTK_DATA_DIR}/smtk/simple.json"
  )

  add_test(
    NAME displayMultiBlockModel
    COMMAND
      $<TARGET_FILE:displayMultiBlockModel>
      "${SMTK_DATA_DIR}/smtk/simple.json"
  )
endif()

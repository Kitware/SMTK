if(SMTK_DATA_DIR)
  add_executable(smtkReadFileTest smtkReadFile.cxx)
  target_link_libraries(smtkReadFileTest
    smtkCore
    smtkVTKSession
    )
  add_test(
    NAME smtkReadFileTest
    COMMAND
      $<TARGET_FILE:smtkReadFileTest>
      "${SMTK_DATA_DIR}/model/3d/exodus/disk_out_ref.smtk"
  )
set_tests_properties( smtkReadFileTest PROPERTIES LABELS "VTKSession" )

endif()

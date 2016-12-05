if(SMTK_DATA_DIR)
  add_executable(entityGroupEx2Test entityGroupEx2.cxx)
  target_link_libraries(entityGroupEx2Test
    smtkCore
    smtkExodusSession
    )
  add_test(
    NAME entityGroupEx2Test
    COMMAND 
      $<TARGET_FILE:entityGroupEx2Test>
      "${SMTK_DATA_DIR}/model/3d/exodus/disk_out_ref.ex2"
  )


endif()

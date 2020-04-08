set (unit_tests
)

set (unit_tests_which_require_data
  ImportMultipleFiles.cxx
)

set(external_libs ${Boost_LIBRARIES} ${VTK_LIBRARIES} vtkSMTKSourceExt smtkIOVTK)

smtk_unit_tests(
  LABEL "VTKSession"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES smtkCore smtkVTKSession smtkCoreModelTesting ${external_libs}
)

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

add_executable(browseModel MACOSX_BUNDLE browseModel.cxx)
qt4_use_modules(browseModel Core Gui)
target_link_libraries(browseModel SMTKCore SMTKCoreModelTesting QtSMTK)
if (NOT WIN32 AND SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/ReadMe.mkd)
  add_test(
    NAME browseModel
    COMMAND
      $<TARGET_FILE:browseModel>
      "${SMTK_DATA_DIR}/smtk/simple.json"
      0xffffffff
  )
endif()

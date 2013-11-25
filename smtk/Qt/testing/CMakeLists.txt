add_executable(browseModel MACOSX_BUNDLE browseModel.cxx)
qt4_use_modules(browseModel Core Gui)
target_link_libraries(browseModel SMTKCore QtSMTK)
add_test(
  NAME browseModel
  COMMAND
    $<TARGET_FILE:browseModel>
    "${SMTK_SOURCE_DIR}/smtk/model/testing/cube.json"
    0xffffffff
)

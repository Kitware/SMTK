qt4_wrap_ui(UI_BUILT_SOURCES ModelBrowser.ui)
add_executable(browseModel MACOSX_BUNDLE
  browseModel.cxx
  ModelBrowser.cxx
  ${UI_BUILT_SOURCES})
set_target_properties(
  browseModel PROPERTIES AUTOMOC TRUE
  INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/bin")
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

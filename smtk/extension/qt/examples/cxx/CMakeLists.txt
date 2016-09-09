##
## Create browseModel to test browsing model entity tree:

qt4_wrap_ui(browseModel_UI_BUILT_SOURCES ModelBrowser.ui)

add_executable(browseModel MACOSX_BUNDLE
  browseModel.cxx
  ModelBrowser.cxx
  ${browseModel_UI_BUILT_SOURCES})
target_link_libraries(browseModel
  smtkCore
  smtkCoreModelTesting
  smtkQtExt
)

set_target_properties(
  browseModel PROPERTIES AUTOMOC TRUE
  INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/bin")

#qt4_use_modules(browseModel Core Gui)

##
## Create an application to test attribute editing

#add in the attribute preview executable
add_executable(qtAttributePreview MACOSX_BUNDLE qtAttributePreview.cxx)
target_link_libraries(qtAttributePreview
  smtkQtExt
)

##
## Test attribute item-widget factory

add_executable(testItemFactory MACOSX_BUNDLE testItemFactory.cxx)
target_link_libraries(testItemFactory PUBLIC smtkQtExt)
add_test(
  NAME testItemFactory
  COMMAND $<TARGET_FILE:testItemFactory>
)

##
## Test model browsing (for crashes only, not behavior)

if (NOT WIN32 AND SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/ReadMe.mkd)
  add_test(
    NAME browseModel
    COMMAND
      $<TARGET_FILE:browseModel>
      "${SMTK_DATA_DIR}/smtk/simple.json"
      0xffffffff
  )
endif()

##
## browse model and mesh (created from the model)

add_executable(browseMesh MACOSX_BUNDLE
  browseMesh.cxx
  ModelBrowser.cxx
  ${browseModel_UI_BUILT_SOURCES})

set_target_properties(
  browseMesh PROPERTIES AUTOMOC TRUE
  INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/bin")

# qt4_use_modules(browseMesh Core Gui)

target_link_libraries(browseMesh smtkCore smtkCoreModelTesting smtkQtExt)

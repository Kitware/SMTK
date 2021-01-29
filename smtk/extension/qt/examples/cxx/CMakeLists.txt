##
## Create browseModel to test browsing model entity tree:

set(CMAKE_AUTOUIC 1)

add_executable(browseModel MACOSX_BUNDLE
  browseModel.cxx
  ModelBrowser.cxx
  ModelBrowser.ui)
target_link_libraries(browseModel
  smtkCore
  smtkCoreModelTesting
  smtkQtExt
)

set_target_properties(
  browseModel PROPERTIES AUTOMOC TRUE
  INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/bin")

##
## Create an application to test attribute editing
add_executable(qtAttributePreview MACOSX_BUNDLE qtAttributePreview.cxx)

# If VTK session enabled, add settings to load model files
set(vtk_session_lib)
if (SMTK_ENABLE_VTK_SUPPORT)
  target_compile_definitions(qtAttributePreview PRIVATE VTK_SESSION)
  set(vtk_session_lib "smtkVTKSession")
endif()

target_link_libraries(qtAttributePreview
  smtkCore
  smtkQtExt
  ${vtk_session_lib}
)

##
## Create an application to open operation dialog
add_executable(qtOperationPreview MACOSX_BUNDLE qtOperationPreview.cxx)

# If VTK session enabled, add settings to load model files
set(vtk_session_lib)
if (SMTK_ENABLE_VTK_SUPPORT)
  target_compile_definitions(qtOperationPreview PRIVATE VTK_SESSION)
  set(vtk_session_lib "smtkVTKSession")
endif()

target_link_libraries(qtOperationPreview
  smtkCore
  smtkQtExt
  ${vtk_session_lib}
)

##
## Test model browsing (for crashes only, not behavior)

if (NOT WIN32 AND SMTK_DATA_DIR)
  add_test(
    NAME browseModel
    COMMAND
      $<TARGET_FILE:browseModel>
      "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json"
      "test"
  )
endif()

# ==============================================================================
# Source
# ==============================================================================
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Qt UI files
# ------------
set(TE_UI_FORMS
  ui/AnalysisForm.ui
  ui/AttDefInformation.ui
  ui/AttributeBrowser.ui
  ui/CategoriesForm.ui
  ui/AttDefDialog.ui
  ui/InputDialog.ui
  ui/ItemDefDialog.ui
  ui/ItemDefinitionForm.ui
  ui/PreviewPanel.ui
  ui/TemplateEditorMain.ui)

smtk_qt_wrap_ui(TE_UIC_SOURCE  ${TE_UI_FORMS})

# Source files
# ------------
set(TE_SOURCE
  AbstractDataModel.h
  AbstractDataModel.cxx
  AttDefDataModel.h
  AttDefDataModel.cxx
  AttDefDialog.h
  AttDefDialog.cxx
  AttDefInformation.h
  AttDefInformation.cxx
  AttributeBrowser.cxx
  AttributeBrowser.h
  InputDialog.cxx
  InputDialog.h
  ItemDefDialog.h
  ItemDefDialog.cxx
  ItemDefinitionsDataModel.h
  ItemDefinitionsDataModel.cxx
  ItemDefinitionHelper.h
  main.cxx
  PreviewPanel.h
  PreviewPanel.cxx
  AttributeProperties.h
  TemplateEditorMain.cxx
  TemplateEditorMain.h
  ${TE_UIC_SOURCE})

# ==============================================================================
# Application
# ==============================================================================

add_executable(TemplateEditor MACOSX_BUNDLE ${TE_SOURCE})
set_target_properties(TemplateEditor PROPERTIES AUTOMOC TRUE
  INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/bin")
target_link_libraries(TemplateEditor
  LINK_PRIVATE
  smtkCore
  smtkQtExt)

# ==============================================================================
# Install
# ==============================================================================

install(TARGETS TemplateEditor
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin      # Required for MACOSX_BUNDLE
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

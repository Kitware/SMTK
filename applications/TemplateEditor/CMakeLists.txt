# ==============================================================================
# Source
# ==============================================================================
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

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
  ui/ItemDefGroupForm.ui
  ui/ItemDefinitionForm.ui
  ui/ItemDefRefForm.ui
  ui/ItemDefStringForm.ui
  ui/ItemDefValueForm.ui
  ui/PreviewPanel.ui
  ui/TemplateEditorMain.ui)

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
  AttDefTreeView.h
  AttributeBrowser.cxx
  AttributeBrowser.h
  HandlerGroup.h
  HandlerGroup.cxx
  HandlerItemDef.h
  HandlerItemDef.cxx
  HandlerValue.h
  HandlerValue.cxx
  HandlerVoid.h
  HandlerVoid.cxx
  DataModelContainers.h
  InputDialog.cxx
  InputDialog.h
  ItemDefDialog.h
  ItemDefDialog.cxx
  ItemDefDataModel.h
  ItemDefDataModel.cxx
  main.cxx
  PreviewPanel.h
  PreviewPanel.cxx
  TemplateEditorMain.cxx
  TemplateEditorMain.h)

set(CMAKE_AUTOUIC 1)

# ==============================================================================
# Application
# ==============================================================================

add_executable(TemplateEditor MACOSX_BUNDLE ${TE_SOURCE} ${TE_UI_FORMS})
set_target_properties(TemplateEditor PROPERTIES AUTOMOC TRUE
  INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/bin")
target_link_libraries(TemplateEditor
  LINK_PRIVATE
  smtkCore
  smtkQtExt
  smtkPolygonSession
)
target_include_directories(TemplateEditor
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}")
set_property(TARGET TemplateEditor APPEND
  PROPERTY
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui")

# ==============================================================================
# Install
# ==============================================================================

install(TARGETS TemplateEditor
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION Applications      # Required for MACOSX_BUNDLE
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

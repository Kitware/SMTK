find_package(Qt5 REQUIRED COMPONENTS Core Svg Widgets)

# set up sources to build
set(QAttrLibSrcs
  qtAvailableOperations.cxx
  qtUIManager.cxx
  qtAttribute.cxx
  qtAttributeEditorDialog.cxx
  qtAttributeItemInfo.cxx
  qtBadgeActionToggle.cxx
  qtBaseAttributeView.cxx
  qtBaseView.cxx
  qtCategorySelectorView.cxx
  qtCheckItemComboBox.cxx
  qtCollapsibleGroupWidget.cxx
  qtComponentItem.cxx
  qtDescriptivePhraseDelegate.cxx
  qtDescriptivePhraseModel.cxx
  qtDoubleClickButton.cxx
  qtGroupView.cxx
  qtAnalysisView.cxx
  qtAssociationView.cxx
  qtAssociation2ColumnWidget.cxx
  qtAttributeView.cxx
  qtInstancedView.cxx
  qtComponentAttributeView.cxx
  qtOperationAction.cxx
  qtOperationDialog.cxx
  qtOperationLauncher.cxx
  qtOperationPalette.cxx
  qtOperationTypeModel.cxx
  qtOperationTypeView.cxx
  qtOperationView.cxx
  qtReferenceItem.cxx
  qtReferenceItemEditor.cxx
  qtReferenceTree.cxx
  qtResourceItem.cxx
  qtResourceBrowser.cxx
  qtResourceBrowserP.cxx
  qtSelectorView.cxx
  qtSimpleExpressionView.cxx
  qtSMTKCallObserversOnMainThreadBehavior.cxx
  qtViewInfoDialog.cxx
  qtFileItem.cxx
  qtTableWidget.cxx
  qtToolPaletteLayout.cxx
  qtColorButton.cxx
  qtLineEdit.cxx
  qtDoubleLineEdit.cxx
  qtDoubleUnitsLineEdit.cxx
  qtViewRegistrar.cxx
  qtWorkletModel.cxx
  qtWorkletPalette.cxx

  qtItem.cxx
  qtDiscreteValueEditor.cxx
  qtInputsItem.cxx
  qtInfixExpressionEditor.cxx
  qtInfixExpressionEditorRow.cxx
  qtGroupItem.cxx
  qtVoidItem.cxx
  qtNewAttributeWidget.cxx
  qtOverlay.cxx
  qtTimeZoneRegionModel.cxx
  qtTimeZoneSelectWidget.cxx
  qtSMTKUtilities.cxx
  qtDateTimeItem.cxx

  MembershipBadge.cxx
  RedirectOutput.cxx
  SVGIconEngine.cxx
  TypeAndColorBadge.cxx
  VisibilityBadge.cxx

  task/qtBaseTaskNode.cxx
  task/qtDefaultTaskNode.cxx
  task/qtDefaultTaskNode1.cxx
  task/qtTaskArc.cxx
  task/qtTaskEditor.cxx
  task/qtTaskScene.cxx
  task/qtTaskView.cxx
  task/qtTaskViewConfiguration.cxx
)

set(QAttrLibUIs
  qtAssociationView.ui
  qtAttributeAssociation.ui
  qtAttributeEditorWidget.ui
  qtCollapsibleGroupWidgetInternals.ui
  qtNewAttributeWidget.ui
  qtTimeZoneSelectWidget.ui
  qtViewInfoDialog.ui
  task/DefaultTaskNode.ui
  task/DefaultTaskNode1.ui
)


set(QAttrLibMocHeaders
  qtAvailableOperations.h
  qtUIManager.h
  qtAssociationWidget.h
  qtAssociation2ColumnWidget.h
  qtAttribute.h
  qtAttributeEditorDialog.h
  qtBaseAttributeView.h
  qtBaseView.h
  qtCategorySelectorView.h
  qtCheckItemComboBox.h
  qtCollapsibleGroupWidget.h
  qtComponentItem.h
  qtEmittingStringBuffer.h
  qtDescriptivePhraseDelegate.h
  qtDescriptivePhraseModel.h
  qtDoubleClickButton.h
  qtGroupView.h
  qtAnalysisView.h
  qtAssociationView.h
  qtAttributeView.h
  qtInstancedView.h
  qtComponentAttributeView.h
  qtNotEditableDelegate.h
  qtOperationAction.h
  qtOperationDialog.h
  qtOperationLauncher.h
  qtOperationPalette.h
  qtOperationTypeModel.h
  qtOperationTypeView.h
  qtOperationView.h
  qtReferenceItem.h
  qtReferenceItemEditor.h
  qtReferenceItemData.h
  qtReferenceTree.h
  qtReferenceTreeData.h
  qtRegexDelegate.h
  qtResourceItem.h
  qtResourceBrowser.h
  qtSelectorView.h
  qtSimpleExpressionView.h
  qtSMTKCallObserversOnMainThreadBehavior.h
  qtViewInfoDialog.h
  qtFileItem.h
  qtTableWidget.h
  qtToolPaletteLayout.h
  qtColorButton.h
  qtLineEdit.h
  qtDoubleLineEdit.h
  qtDoubleUnitsLineEdit.h

  qtItem.h
  qtDiscreteValueEditor.h
  qtInputsItem.h
  qtInfixExpressionEditor.h
  qtInfixExpressionEditorRow.h
  qtGroupItem.h
  qtVoidItem.h
  qtNewAttributeWidget.h
  qtOverlay.h
  qtDateTimeItem.h
  qtTimeZoneRegionModel.h
  qtTimeZoneSelectWidget.h

  MembershipBadge.h
  SVGIconEngine.h
  VisibilityBadge.h

  task/qtBaseTaskNode.h
  task/qtDefaultTaskNode.h
  task/qtDefaultTaskNode1.h
  task/qtTaskArc.h
  task/qtTaskEditor.h
  task/qtTaskNodeFactory.h
  task/qtTaskScene.h
  task/qtTaskView.h
  task/qtTaskViewConfiguration.h

  qtManager.h
)

set(QAttrLibHeaders
  ${QAttrLibMocHeaders}
  RedirectOutput.h
  qtAttributeItemInfo.h
  qtBadgeActionToggle.h
  qtDirectoryItem.h
  qtDoubleItem.h
  qtIntItem.h
  qtResourceBrowserP.h
  qtSMTKUtilities.h
  qtStringItem.h
  qtTypeDeclarations.h
  qtViewRegistrar.h
  qtWorkletModel.h
  qtWorkletPalette.h
)

# Put the contents of this file in a string in a header, ending in `_json.h`.
smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/ResourcePanelConfiguration.json"
  TYPE "_json"
  HEADER_OUTPUT rpJsonHeader)

# Put the contents of this file in a *function* in a header, ending in `_cpp.h`.
smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/task/PanelConfiguration.json"
  NAME "taskPanelConfiguration"
  TYPE "_cpp"
  HEADER_OUTPUT tpXmlHeader
)

#install the headers
smtk_public_headers(smtkQtExt ${QAttrLibHeaders})

set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTOUIC 1)
set(CMAKE_AUTORCC 1)

add_library(smtkQtExt
  ${QAttrLibSrcs}
  ${QAttrLibMocHeaders}
  ${QAttrLibUIs}
  qtDescriptivePhraseModelIcons.qrc
  qtReferenceItemIcons.qrc
  qtAttributeIcons.qrc
  ${rpJsonHeader}
  ${tpXmlHeader}
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  list(APPEND _extra_private_libraries
    pybind11::embed
  )
endif()

if (SMTK_ENABLE_GRAPHVIZ_SUPPORT)
  list(APPEND _extra_private_libraries
    Graphviz::cgraph
    Graphviz::gvc
  )
endif()

target_compile_definitions(smtkQtExt PRIVATE
  "SMTK_ENABLE_GRAPHVIZ_SUPPORT=$<BOOL:${SMTK_ENABLE_GRAPHVIZ_SUPPORT}>"
)

if (NOT SMTK_ENABLE_OPERATION_THREADS)
  target_compile_definitions(smtkQtExt PRIVATE SINGLE_THREAD)
endif()
target_compile_definitions(smtkQtExt PRIVATE QT_NO_KEYWORDS)

#we need to add the location of the moc files to the include dir for qtsmtk
target_include_directories(smtkQtExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#set smtkQtExt to publicly link to smtkCore
target_link_libraries(smtkQtExt
  LINK_PUBLIC
  smtkCore
  Qt5::Core
  Qt5::Svg
  Qt5::Widgets
  LINK_PRIVATE
  Threads::Threads
  ${_extra_private_libraries}
  )
# add_dependencies(smtkQtExt ${rpJsonTarget})

smtk_export_header(smtkQtExt Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkQtExt)

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()

add_subdirectory(testing)

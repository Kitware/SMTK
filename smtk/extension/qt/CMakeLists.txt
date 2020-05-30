find_package(Qt5 REQUIRED COMPONENTS Core Svg Widgets)

# set up sources to build
set(QAttrLibSrcs
  qtActiveObjects.cxx
  qtAvailableOperations.cxx
  qtUIManager.cxx
  qtAttribute.cxx
  qtAttributeDisplay.cxx
  qtAttributeEditorDialog.cxx
  qtAttributeItemInfo.cxx
  qtBaseAttributeView.cxx
  qtBaseView.cxx
  qtCategorySelectorView.cxx
  qtCheckItemComboBox.cxx
  qtCollapsibleGroupWidget.cxx
  qtComponentItem.cxx
  qtDescriptivePhraseDelegate.cxx
  qtDescriptivePhraseModel.cxx
  qtGroupView.cxx
  qtAnalysisView.cxx
  qtAssociationView.cxx
  qtAssociation2ColumnWidget.cxx
  qtAttributeView.cxx
  qtInstancedView.cxx
  qtModelEntityAttributeView.cxx
  qtReferenceItem.cxx
  qtResourceItem.cxx
  qtReferenceItemComboBox.cxx
  qtResourceBrowser.cxx
  qtResourceBrowserP.cxx
  qtOperationLauncher.cxx
  qtOperationView.cxx
  qtSelectorView.cxx
  qtSimpleExpressionView.cxx
  qtViewInfoDialog.cxx
  qtFileItem.cxx
  qtTableWidget.cxx
  qtColorButton.cxx
  qtLineEdit.cxx
  qtDoubleLineEdit.cxx
  qtViewRegistrar.cxx

  qtItem.cxx
  qtDiscreteValueEditor.cxx
  qtInputsItem.cxx
  qtGroupItem.cxx
  qtVoidItem.cxx
  qtNewAttributeWidget.cxx
  qtOverlay.cxx
  qtTimeZoneRegionModel.cxx
  qtTimeZoneSelectWidget.cxx
  qtViewInterface.cxx
  qtSMTKUtilities.cxx
  qtDateTimeItem.cxx

  MembershipBadge.cxx
  RedirectOutput.cxx
  SVGIconEngine.cxx
  TypeAndColorBadge.cxx
  )

set(QAttrLibUIs
  qtAssociationView.ui
  qtAttributeAssociation.ui
  qtAttributeEditorWidget.ui
  qtCollapsibleGroupWidgetInternals.ui
  qtNewAttributeWidget.ui
  qtTimeZoneSelectWidget.ui
  qtViewInfoDialog.ui
)


set(QAttrLibMocHeaders
  qtActiveObjects.h
  qtAvailableOperations.h
  qtUIManager.h
  qtAssociationWidget.h
  qtAssociation2ColumnWidget.h
  qtAttribute.h
  qtAttributeDisplay.h
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
  qtGroupView.h
  qtAnalysisView.h
  qtAssociationView.h
  qtAttributeView.h
  qtInstancedView.h
  qtModelEntityAttributeView.h
  qtNotEditableDelegate.h
  qtReferenceItem.h
  qtReferenceItemComboBox.h
  qtReferenceItemData.h
  qtResourceItem.h
  qtResourceBrowser.h
  qtOperationLauncher.h
  qtOperationView.h
  qtSelectorView.h
  qtSimpleExpressionView.h
  qtViewInfoDialog.h
  qtFileItem.h
  qtTableWidget.h
  qtColorButton.h
  qtLineEdit.h
  qtDoubleLineEdit.h

  qtItem.h
  qtDiscreteValueEditor.h
  qtInputsItem.h
  qtGroupItem.h
  qtVoidItem.h
  qtNewAttributeWidget.h
  qtOverlay.h
  qtDateTimeItem.h
  qtTimeZoneRegionModel.h
  qtTimeZoneSelectWidget.h

  MembershipBadge.h
  SVGIconEngine.h
)

set(QAttrLibHeaders
  ${QAttrLibMocHeaders}
  RedirectOutput.h
  qtAttributeItemInfo.h
  qtDirectoryItem.h
  qtDoubleItem.h
  qtIntItem.h
  qtResourceBrowserP.h
  qtSMTKUtilities.h
  qtStringItem.h
  qtTypeDeclarations.h
  qtViewInterface.h
  qtViewRegistrar.h
)

# put contents of this file in a string in a header, ending _json.h
smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/ResourcePanelConfiguration.json"
  TYPE "_json"
  HEADER_OUTPUT rpJsonHeader)

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
)

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

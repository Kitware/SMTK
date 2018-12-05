find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# set up sources to build
set(QAttrLibSrcs
  qtActiveObjects.cxx
  qtAvailableOperations.cxx
  qtUIManager.cxx
  qtAttribute.cxx
  qtAttributeDisplay.cxx
  qtBaseView.cxx
  qtCategorySelectorView.cxx
  qtCheckItemComboBox.cxx
  qtCollapsibleGroupWidget.cxx
  qtComponentItem.cxx
  qtDescriptivePhraseDelegate.cxx
  qtDescriptivePhraseEditor.cxx
  qtDescriptivePhraseModel.cxx
  qtGroupView.cxx
  qtAttributeView.cxx
  qtInstancedView.cxx
  qtModelEntityAttributeView.cxx
  qtReferenceItem.cxx
  qtResourceItem.cxx
  qtResourceBrowser.cxx
  qtOperationLauncher.cxx
  qtOperationView.cxx
  #qtModelView.cxx
  qtSelectorView.cxx
  qtSimpleExpressionView.cxx
  qtViewInfoDialog.cxx
  qtFileItem.cxx
  qtAttributeRefItem.cxx
  qtTableWidget.cxx
  qtAssociationWidget.cxx
  qtReferencesWidget.cxx
  qtColorButton.cxx

  qtItem.cxx
  qtDiscreteValueEditor.cxx
  qtInputsItem.cxx
  qtGroupItem.cxx
  qtVoidItem.cxx
  qtModelView.cxx
  qtModelPanel.cxx
  qtNewAttributeWidget.cxx
  qtOverlay.cxx
  qtModelOperationWidget.cxx
  qtOperationDockWidget.cxx
  qtTimeZoneRegionModel.cxx
  qtTimeZoneSelectWidget.cxx
  qtViewInterface.cxx
  qtSMTKUtilities.cxx
  qtDateTimeItem.cxx
  RedirectOutput.cxx
  )

set(QAttrLibUIs
  qtAttributeAssociation.ui
  qtCollapsibleGroupWidgetInternals.ui
  qtModelPanel.ui
  qtNewAttributeWidget.ui
  qtTimeZoneSelectWidget.ui
  qtViewInfoDialog.ui
)


set(QAttrLibMocHeaders
  qtActiveObjects.h
  qtAvailableOperations.h
  qtUIManager.h
  qtAttribute.h
  qtAttributeDisplay.h
  qtBaseView.h
  qtCategorySelectorView.h
  qtCheckItemComboBox.h
  qtCollapsibleGroupWidget.h
  qtComponentItem.h
  qtEmittingStringBuffer.h
  qtDescriptivePhraseDelegate.h
  qtDescriptivePhraseEditor.h
  qtDescriptivePhraseModel.h
  qtGroupView.h
  qtAttributeView.h
  qtInstancedView.h
  qtModelEntityAttributeView.h
  qtReferenceItem.h
  qtReferenceItemData.h
  qtResourceItem.h
  qtResourceBrowser.h
  qtOperationLauncher.h
  qtOperationView.h
  #qtModelView.h
  qtSelectorView.h
  qtSimpleExpressionView.h
  qtViewInfoDialog.h
  qtFileItem.h
  qtAttributeRefItem.h
  qtTableWidget.h
  qtAssociationWidget.h
  qtReferencesWidget.h
  qtColorButton.h

  qtItem.h
  qtDiscreteValueEditor.h
  qtInputsItem.h
  qtGroupItem.h
  qtVoidItem.h
  qtModelView.h
  qtModelPanel.h
  qtNewAttributeWidget.h
  qtOverlay.h
  qtModelOperationWidget.h
  qtOperationDockWidget.h
  qtDateTimeItem.h
  qtTimeZoneRegionModel.h
  qtTimeZoneSelectWidget.h
)

set(QAttrLibHeaders
  ${QAttrLibMocHeaders}
  qtDirectoryItem.h
  qtDoubleItem.h
  qtIntItem.h
  qtStringItem.h
  qtViewInterface.h
  qtSMTKUtilities.h
  RedirectOutput.h
)

#install the headers
smtk_public_headers(smtkQtExt ${QAttrLibHeaders})

qt5_wrap_cpp(MOC_BUILT_SOURCES ${QAttrLibMocHeaders})
qt5_wrap_ui(SMTKQTEXT_UI_BUILT_SOURCES ${QAttrLibUIs})
qt5_add_resources(RCS_RESOURCES
  qtDescriptivePhraseModelIcons.qrc
  qtReferenceItemIcons.qrc
  qtAttributeIcons.qrc
)

source_group("Qt_Source" FILES ${QAttrLibSrcs} ${QAttrLibUIs})
source_group("Qt_Generated" FILES ${MOC_BUILT_SOURCES} ${SMTKQTEXT_UI_BUILT_SOURCES} ${RCS_RESOURCES})
source_group("Qt_Header" FILES ${QAttrLibMocHeaders})

add_library(smtkQtExt
  ${QAttrLibSrcs}
  ${MOC_BUILT_SOURCES}
  ${SMTKQTEXT_UI_BUILT_SOURCES}
  ${RCS_RESOURCES}
)

#we need to add the location of the moc files to the include dir for qtsmtk
target_include_directories(smtkQtExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#set smtkQtExt to publicly link to smtkCore
target_link_libraries(smtkQtExt LINK_PUBLIC
  smtkCore
  Qt5::Core
  Qt5::Widgets
  )
smtk_export_header(smtkQtExt Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkQtExt)

if (SMTK_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()

add_subdirectory(testing)

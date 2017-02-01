include(${QT_USE_FILE})

# set up sources to build
set(QAttrLibSrcs
  qtUIManager.cxx
  qtSelectionManager.cxx
  qtAttribute.cxx
  qtAttributeDisplay.cxx
  qtAttributeItemWidgetFactory.cxx
  qtBaseView.cxx
  qtCheckItemComboBox.cxx
  qtCollapsibleGroupWidget.cxx
  qtEntityItemDelegate.cxx
  qtEntityItemEditor.cxx
  qtEntityItemModel.cxx
  qtGroupView.cxx
  qtAttributeView.cxx
  qtInstancedView.cxx
  qtOperatorView.cxx
  #qtModelView.cxx
  qtSimpleExpressionView.cxx
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
  qtMeshItem.cxx
  qtMeshSelectionItem.cxx
  qtModelEntityItem.cxx
  qtModelOperationWidget.cxx
  qtOperatorDockWidget.cxx
  qtViewInterface.cxx
  qtSMTKUtilities.cxx
  )

set(QAttrLibUIs
  qtAttributeAssociation.ui
  qtCollapsibleGroupWidgetInternals.ui
  qtModelPanel.ui
  qtNewAttributeWidget.ui
)


set(QAttrLibMocHeaders
  qtUIManager.h
  qtSelectionManager.h
  qtAttribute.h
  qtAttributeDisplay.h
  qtBaseView.h
  qtCheckItemComboBox.h
  qtCollapsibleGroupWidget.h
  qtEntityItemDelegate.h
  qtEntityItemEditor.h
  qtEntityItemModel.h
  qtGroupView.h
  qtAttributeView.h
  qtInstancedView.h
  qtOperatorView.h
  #qtModelView.h
  qtSimpleExpressionView.h
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
  qtMeshItem.h
  qtMeshSelectionItem.h
  qtModelEntityItem.h
  qtModelOperationWidget.h
  qtOperatorDockWidget.h
)

set(QAttrLibHeaders
  ${QAttrLibMocHeaders}
  qtAttributeItemWidgetFactory.h
  qtViewInterface.h
  qtSMTKUtilities.h
)

#install the headers
smtk_public_headers(${QAttrLibHeaders})

qt4_wrap_cpp(MOC_BUILT_SOURCES ${QAttrLibMocHeaders})
qt4_wrap_ui(SMTKQTEXT_UI_BUILT_SOURCES ${QAttrLibUIs})
qt4_add_resources(RCS_RESOURCES qtEntityItemModelIcons.qrc qtAttributeIcons.qrc)

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

#set smtkQtExt to link to qtGUI and include dirs
#qt4_use_modules(smtkQtExt LINK_PUBLIC Core Gui)

#set smtkQtExt to publicly link to smtkCore
target_link_libraries(smtkQtExt LINK_PUBLIC
  smtkCore
  ${QT_QTGUI_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  )
smtk_export_header(smtkQtExt Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkQtExt)

if (SMTK_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()

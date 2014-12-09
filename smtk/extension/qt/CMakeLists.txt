# set up sources to build
set(QAttrLibSrcs
    qtUIManager.cxx
    qtAttribute.cxx
    qtBaseView.cxx
    qtCheckItemComboBox.cxx
    qtEntityItemDelegate.cxx
    qtEntityItemEditor.cxx
    qtEntityItemModel.cxx
    qtGroupView.cxx
    qtRootView.cxx
    qtAttributeView.cxx
    qtInstancedView.cxx
    #qtModelEntityView.cxx
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
    qtModelEntityItem.cxx
    qtModelOperationWidget.cxx
  )

  set(QAttrLibUIs
    qtAttributeAssociation.ui
    qtModelPanel.ui
    qtNewAttributeWidget.ui
    )


set(QAttrLibHeaders
    qtUIManager.h
    qtAttribute.h
    qtBaseView.h
    qtCheckItemComboBox.h
    qtEntityItemDelegate.h
    qtEntityItemEditor.h
    qtEntityItemModel.h
    qtGroupView.h
    qtRootView.h
    qtAttributeView.h
    qtInstancedView.h
    #qtModelEntityView.h
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
    qtModelEntityItem.h
    qtModelOperationWidget.h
  )

#install the headers
smtk_public_headers(${QAttrLibHeaders})

qt4_wrap_cpp(MOC_BUILT_SOURCES ${QAttrLibHeaders})
qt4_wrap_ui(QtSMTK_UI_BUILT_SOURCES ${QAttrLibUIs})
qt4_add_resources(RCS_RESOURCES qtEntityItemModelIcons.qrc qtAttributeIcons.qrc)

source_group("Qt_Source" FILES ${QAttrLibSrcs} ${QAttrLibUIs})
source_group("Qt_Generated" FILES ${MOC_BUILT_SOURCES} ${QtSMTK_UI_BUILT_SOURCES} ${RCS_RESOURCES})
source_group("Qt_Header" FILES ${QAttrLibMocHeaders})

add_library(QtSMTK
            ${QAttrLibSrcs}
            ${MOC_BUILT_SOURCES}
            ${QtSMTK_UI_BUILT_SOURCES}
            ${RCS_RESOURCES}
            )

# On Mac OS X, set the directory included as part of the installed library's path:
set_target_properties(QtSMTK PROPERTIES
                      INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")

#we need to add the location of the moc files to the include dir for qtsmtk
target_include_directories(QtSMTK PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#set QtSMTK to link to qtGUI and include dirs
qt4_use_modules(QtSMTK LINK_PUBLIC Core Gui)

#set QtSMTK to publicly link to SMTKCore
target_link_libraries(QtSMTK LINK_PUBLIC SMTKCore)
smtk_export_header(QtSMTK QtSMTKExports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(QtSMTK DEPENDS SMTKCore)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

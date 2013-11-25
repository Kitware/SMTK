# set up sources to build
set(QAttrLibSrcs
    qtUIManager.cxx
    qtAttribute.cxx
    qtBaseView.cxx
    qtEntityItemModel.cxx
    qtGroupView.cxx
    qtRootView.cxx
    qtAttributeView.cxx
    qtInstancedView.cxx
    qtModelEntityView.cxx
    qtSimpleExpressionView.cxx
    qtFileItem.cxx
    qtAttributeRefItem.cxx
    qtTableWidget.cxx
    qtAssociationWidget.cxx
    qtReferencesWidget.cxx
    qtColorButton.cxx

    qtItem.cxx
    qtComboItem.cxx
    qtInputsItem.cxx
    qtGroupItem.cxx
    qtVoidItem.cxx
  )

  set(QAttrLibUIs
    qtAttributeAssociation.ui
    )


set(QAttrLibHeaders
    qtUIManager.h
    qtAttribute.h
    qtBaseView.h
    qtEntityItemModel.h
    qtGroupView.h
    qtRootView.h
    qtAttributeView.h
    qtInstancedView.h
    qtModelEntityView.h
    qtSimpleExpressionView.h
    qtFileItem.h
    qtAttributeRefItem.h
    qtTableWidget.h
    qtAssociationWidget.h
    qtReferencesWidget.h
    qtColorButton.h

    qtItem.h
    qtComboItem.h
    qtInputsItem.h
    qtGroupItem.h
    qtVoidItem.h
  )

#install the headers
smtk_public_headers(${QAttrLibHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

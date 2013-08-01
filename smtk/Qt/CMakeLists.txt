# set up sources to build
set(QAttrLibSrcs
    qtUIManager.cxx
    qtAttribute.cxx
    qtSection.cxx
    qtGroupSection.cxx
    qtRootSection.cxx
    qtAttributeSection.cxx
    qtInstancedSection.cxx
    qtModelEntitySection.cxx
    qtSimpleExpressionSection.cxx
    qtFileItem.cxx
    qtAttributeRefItem.cxx
    qtTableWidget.cxx
    qtAssociationWidget.cxx
    qtReferencesWidget.cxx
    qtColorButton.cxx
    qtColorItem.cxx

    qtItem.cxx
    qtComboItem.cxx
    qtInputsItem.cxx
    qtGroupItem.cxx
  )

  set(QAttrLibUIs
    qtAttributeAssociation.ui
    )


set(QAttrLibMocHeaders
    qtUIManager.h
    qtAttribute.h
    qtSection.h
    qtGroupSection.h
    qtRootSection.h
    qtAttributeSection.h
    qtInstancedSection.h
    qtModelEntitySection.h
    qtSimpleExpressionSection.h
    qtFileItem.h
    qtAttributeRefItem.h
    qtTableWidget.h
    qtAssociationWidget.h
    qtReferencesWidget.h
    qtColorButton.h
    qtColorItem.h

    qtItem.h
    qtComboItem.h
    qtInputsItem.h
    qtGroupItem.h
  )

#install the headers
smtk_public_headers(${QAttrLibMocHeaders})


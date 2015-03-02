project(SMTK_Attribute)

set(attributeHeaders
  Attribute.h
  Definition.h
  DirectoryItem.h
  DirectoryItemDefinition.h
  DoubleItem.h
  DoubleItemDefinition.h
  FileItem.h
  FileItemDefinition.h
  GroupItem.h
  GroupItemDefinition.h
  IntItem.h
  IntItemDefinition.h
  Item.h
  ItemDefinition.h
  System.h
  MeshSelectionItem.h
  MeshSelectionItemDefinition.h
  ModelEntityItem.h
  ModelEntityItemDefinition.h
  RefItem.h
  RefItemDefinition.h
  SearchStyle.h
  StringItem.h
  StringItemDefinition.h
  ValueItem.h
  ValueItemDefinition.h
  ValueItemDefinitionTemplate.h
  ValueItemTemplate.h
  VoidItem.h
  VoidItemDefinition.h
  )

set(attributeSrcs
  Attribute.cxx
  Definition.cxx
  DirectoryItem.cxx
  DirectoryItemDefinition.cxx
  DoubleItem.cxx
  DoubleItemDefinition.cxx
  FileItem.cxx
  FileItemDefinition.cxx
  GroupItem.cxx
  GroupItemDefinition.cxx
  IntItem.cxx
  IntItemDefinition.cxx
  Item.cxx
  ItemDefinition.cxx
  System.cxx
  MeshSelectionItem.cxx
  MeshSelectionItemDefinition.cxx
  ModelEntityItem.cxx
  ModelEntityItemDefinition.cxx
  RefItem.cxx
  RefItemDefinition.cxx
  StringItem.cxx
  StringItemDefinition.cxx
  ValueItem.cxx
  ValueItemDefinition.cxx
  VoidItem.cxx
  VoidItemDefinition.cxx
)

#install the headers
smtk_public_headers(${attributeHeaders})


if(SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif(SMTK_ENABLE_TESTING)


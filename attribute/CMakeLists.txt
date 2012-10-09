project(Slctk_Attribute)

set(headers
  Attribute.h
  AttributeRefItem.h
  AttributeRefItemDefinition.h
  AttributeSection.h
  Item.h
  ItemDefinition.h
  Definition.h
  DirectoryItem.h
  DirectoryItemDefinition.h
  DoubleItem.h
  DoubleItemDefinition.h
  FileItem.h
  FileItemDefinition.h
  GroupItem.h
  GroupItemDefinition.h
  GroupSection.h
  InstancedSection.h
  IntItem.h
  IntItemDefinition.h
  Manager.h
  ModelEntitySection.h
  RootSection.h
  Section.h
  SimpleExpressionSection.h
  StringItem.h
  StringItemDefinition.h
  ValueItem.h
  ValueItemDefinition.h
  ValueItemDefinitionTemplate.h
  ValueItemTemplate.h
  VoidItem.h
  VoidItemDefinition.h
  XmlV1StringReader.h
  XmlV1StringWriter.h
  )

set(attribute_srcs
  Attribute.cxx
  AttributeRefItem.cxx
  AttributeRefItemDefinition.cxx
  AttributeSection.cxx
  Item.cxx
  ItemDefinition.cxx
  Definition.cxx
  DirectoryItem.cxx
  DirectoryItemDefinition.cxx
  DoubleItem.cxx
  DoubleItemDefinition.cxx
  FileItem.cxx
  FileItemDefinition.cxx
  GroupItem.cxx
  GroupItemDefinition.cxx
  GroupSection.cxx
  InstancedSection.cxx
  IntItem.cxx
  IntItemDefinition.cxx
  Manager.cxx
  ModelEntitySection.cxx
  RootSection.cxx
  Section.cxx
  SimpleExpressionSection.cxx
  StringItem.cxx
  StringItemDefinition.cxx
  ValueItem.cxx
  ValueItemDefinition.cxx
  VoidItem.cxx
  VoidItemDefinition.cxx
  XmlV1StringWriter.cxx
  XmlV1StringReader.cxx
)

add_library(SlctkAttribute ${attribute_srcs} ${headers})
target_link_libraries(SlctkAttribute)

slctk_set_includes(SlctkAttribute ${Slctk_Attribute_INCLUDE_DIRS})

#create the export header symbol exporting
slctk_export_header(SlctkAttribute AttributeExports.h)

#install the library
slctk_install_library(SlctkAttribute)

#install the headers
slctk_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(PACKAGE SlctkAttribute)
export(TARGETS SlctkAttribute FILE SlctkAttribute-exports.cmake)

if(Slctk_ENABLE_TESTING)
  add_subdirectory(Testing)
endif(Slctk_ENABLE_TESTING)


project(Slctk_Attribute)

set(headers
  Attribute.h
  AttributeRefItem.h
  AttributeRefItemDefinition.h
  Cluster.h
  Item.h
  ItemDefinition.h
  Definition.h
  DoubleItem.h
  DoubleItemDefinition.h
  GroupItem.h
  GroupItemDefinition.h
  IntItem.h
  IntItemDefinition.h
  Manager.h
  StringItem.h
  StringItemDefinition.h
  ValueItem.h
  ValueItemDefinition.h
  ValueItemDefinitionTemplate.h
  ValueItemTemplate.h
  )

set(attribute_srcs
  Attribute.cxx
  AttributeRefItem.cxx
  AttributeRefItemDefinition.cxx
  Cluster.cxx
  Item.cxx
  ItemDefinition.cxx
  Definition.cxx
  DoubleItem.cxx
  DoubleItemDefinition.cxx
  GroupItem.cxx
  GroupItemDefinition.cxx
  IntItem.cxx
  IntItemDefinition.cxx
  Manager.cxx
  StringItem.cxx
  StringItemDefinition.cxx
  ValueItem.cxx
  ValueItemDefinition.cxx
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


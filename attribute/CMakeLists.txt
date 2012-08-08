project(Slctk_Attribute)

set(headers
  Attribute.h
  AttributeReferenceComponent.h
  AttributeReferenceComponentDefinition.h
  Cluster.h
  Component.h
  ComponentDefinition.h
  Definition.h
  DoubleComponent.h
  DoubleComponentDefinition.h
  GroupComponent.h
  GroupComponentDefinition.h
  IntegerComponent.h
  IntegerComponentDefinition.h
  Manager.h
  StringComponent.h
  StringComponentDefinition.h
  ValueComponent.h
  ValueComponentDefinition.h
  ValueComponentDefinitionTemplate.h
  ValueComponentTemplate.h
  )

set(attribute_srcs
  Attribute.cxx
  AttributeReferenceComponent.cxx
  AttributeReferenceComponentDefinition.cxx
  Cluster.cxx
  Component.cxx
  ComponentDefinition.cxx
  Definition.cxx
  DoubleComponent.cxx
  DoubleComponentDefinition.cxx
  GroupComponent.cxx
  GroupComponentDefinition.cxx
  IntegerComponent.cxx
  IntegerComponentDefinition.cxx
  Manager.cxx
  StringComponent.cxx
  StringComponentDefinition.cxx
  ValueComponent.cxx
  ValueComponentDefinition.cxx
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

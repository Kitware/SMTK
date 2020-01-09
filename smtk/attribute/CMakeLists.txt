set(jsonAttributeHeaders
  json/jsonAttribute.h
  json/jsonComponentItem.h
  json/jsonComponentItemDefinition.h
  json/jsonResource.h
  json/jsonDateTimeItem.h
  json/jsonDateTimeItemDefinition.h
  json/jsonDirectoryItem.h
  json/jsonDirectoryItemDefinition.h
  json/jsonDoubleItem.h
  json/jsonDoubleItemDefinition.h
  json/jsonDefinition.h
  json/jsonFileItem.h
  json/jsonFileItemDefinition.h
  json/jsonFileSystemItem.h
  json/jsonFileSystemItemDefinition.h
  json/jsonGroupItem.h
  json/jsonGroupItemDefinition.h
  json/jsonHelperFunction.h
  json/jsonIntItem.h
  json/jsonIntItemDefinition.h
  json/jsonItem.h
  json/jsonItemDefinition.h
  json/jsonModelEntityItem.h
  json/jsonModelEntityItemDefinition.h
  json/jsonStringItem.h
  json/jsonStringItemDefinition.h
  json/jsonReferenceItem.h
  json/jsonReferenceItemDefinition.h
  json/jsonResourceItem.h
  json/jsonResourceItemDefinition.h
  json/jsonValueItem.h
  json/jsonValueItemDefinition.h
  json/jsonVoidItem.h
  json/jsonVoidItemDefinition.h
)

set(jsonAttributeSrcs
  json/jsonAttribute.cxx
  json/jsonComponentItem.cxx
  json/jsonComponentItemDefinition.cxx
  json/jsonResource.cxx
  json/jsonDateTimeItem.cxx
  json/jsonDateTimeItemDefinition.cxx
  json/jsonDirectoryItem.cxx
  json/jsonDirectoryItemDefinition.cxx
  json/jsonDoubleItem.cxx
  json/jsonDoubleItemDefinition.cxx
  json/jsonDefinition.cxx
  json/jsonFileItem.cxx
  json/jsonFileItemDefinition.cxx
  json/jsonFileSystemItem.cxx
  json/jsonFileSystemItemDefinition.cxx
  json/jsonGroupItem.cxx
  json/jsonGroupItemDefinition.cxx
  json/jsonHelperFunction.cxx
  json/jsonIntItem.cxx
  json/jsonIntItemDefinition.cxx
  json/jsonItem.cxx
  json/jsonItemDefinition.cxx
  json/jsonModelEntityItem.cxx
  json/jsonModelEntityItemDefinition.cxx
  json/jsonStringItem.cxx
  json/jsonStringItemDefinition.cxx
  json/jsonReferenceItem.cxx
  json/jsonReferenceItemDefinition.cxx
  json/jsonResourceItem.cxx
  json/jsonResourceItemDefinition.cxx
  json/jsonValueItem.cxx
  json/jsonValueItemDefinition.cxx
  json/jsonVoidItem.cxx
  json/jsonVoidItemDefinition.cxx
)

set(attributeHeaders
  ${jsonAttributeHeaders}
  Analyses.h
  Attribute.h
  Categories.h
  ComponentItem.h
  ComponentItemDefinition.h
  Definition.h
  DateTimeItem.h
  DateTimeItemDefinition.h
  DirectoryInfo.h
  DirectoryItem.h
  DirectoryItemDefinition.h
  DoubleItem.h
  DoubleItemDefinition.h
  FileItem.h
  FileItemDefinition.h
  FileSystemItem.h
  FileSystemItemDefinition.h
  GroupItem.h
  GroupItemDefinition.h
  IntItem.h
  IntItemDefinition.h
  Item.h
  ItemDefinition.h
  ModelEntityItem.h
  ModelEntityItemDefinition.h
  PathGrammar.h
  ReferenceItem.h
  ReferenceItemDefinition.h
  Registrar.h
  Resource.h
  ResourceItem.h
  ResourceItemDefinition.h
  SearchStyle.h
  StringItem.h
  StringItemDefinition.h
  Tag.h
  UnsetValueError.h
  ValueItem.h
  ValueItemDefinition.h
  ValueItemDefinitionTemplate.h
  ValueItemTemplate.h
  VoidItem.h
  VoidItemDefinition.h

  operators/Associate.h
  operators/Dissociate.h
  operators/Export.h
  operators/Import.h
  operators/Read.h
  operators/Signal.h
  operators/Write.h
)

set(attributeSrcs
  ${jsonAttributeSrcs}
  Analyses.cxx
  Attribute.cxx
  Categories.cxx
  ComponentItem.cxx
  ComponentItemDefinition.cxx
  DateTimeItem.cxx
  DateTimeItemDefinition.cxx
  Definition.cxx
  DirectoryInfo.cxx
  DirectoryItem.cxx
  DirectoryItemDefinition.cxx
  DoubleItem.cxx
  DoubleItemDefinition.cxx
  FileItem.cxx
  FileItemDefinition.cxx
  FileSystemItem.cxx
  FileSystemItemDefinition.cxx
  GroupItem.cxx
  GroupItemDefinition.cxx
  IntItem.cxx
  IntItemDefinition.cxx
  Item.cxx
  ItemDefinition.cxx
  ModelEntityItem.cxx
  ModelEntityItemDefinition.cxx
  ReferenceItem.cxx
  ReferenceItemDefinition.cxx
  Registrar.cxx
  Resource.cxx
  ResourceItem.cxx
  ResourceItemDefinition.cxx
  StringItem.cxx
  StringItemDefinition.cxx
  ValueItem.cxx
  ValueItemDefinition.cxx
  VoidItem.cxx
  VoidItemDefinition.cxx

  operators/Associate.cxx
  operators/Dissociate.cxx
  operators/Export.cxx
  operators/Import.cxx
  operators/Read.cxx
  operators/Signal.cxx
  operators/Write.cxx
)

#construct operator inputs
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Associate.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Dissociate.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Export.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Import.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Read.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Signal.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Write.sbt" defOpXML)

#install the headers
smtk_public_headers(smtkCore ${attributeHeaders})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if(SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif(SMTK_ENABLE_TESTING)

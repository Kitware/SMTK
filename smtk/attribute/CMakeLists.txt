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

set(attributeOperators
  Associate
  CreateAttribute
  DeleteAttribute
  Dissociate
  EditAttributeItem
  Export
  Import
  Read
  RenameAttribute
  Signal
  Write
)

set(attributeHeaders
  ${jsonAttributeHeaders}
  Analyses.h
  AssociationRule.h
  AssociationRuleFactory.h
  AssociationRuleManager.h
  AssociationRules.h
  Attribute.h
  ComponentItem.h
  ComponentItemDefinition.h
  CopyAssignmentOptions.h
  CustomItem.h
  CustomItemDefinition.h
  Definition.h
  DateTimeItem.h
  DateTimeItemDefinition.h
  DirectoryInfo.h
  DirectoryItem.h
  DirectoryItemDefinition.h
  DoubleItem.h
  DoubleItemDefinition.h
  Evaluator.h
  EvaluatorFactory.h
  EvaluatorManager.h
  FileItem.h
  FileItemDefinition.h
  FileSystemItem.h
  FileSystemItemDefinition.h
  GroupItem.h
  GroupItemDefinition.h
  InfixExpressionEvaluator.h
  IntItem.h
  IntItemDefinition.h
  Item.h
  ItemDefinition.h
  ItemDefinitionManager.h
  ModelEntityItem.h
  ModelEntityItemDefinition.h
  PathGrammar.h
  ReferenceItem.h
  ReferenceItemConstIteratorTemplate.h
  ReferenceItemDefinition.h
  Registrar.h
  Resource.h
  ResourceItem.h
  ResourceItemDefinition.h
  SearchStyle.h
  StringItem.h
  StringItemDefinition.h
  SymbolDependencyStorage.h
  Tag.h
  UnsetValueError.h
  UpdateManager.h
  ValueItem.h
  ValueItemDefinition.h
  ValueItemDefinitionTemplate.h
  ValueItemTemplate.h
  VoidItem.h
  VoidItemDefinition.h
  filter/Action.h
  filter/Attribute.h
  filter/FloatingPointActions.h
  filter/Grammar.h
  filter/GrammarInfo.h
  filter/GrammarInfoActions.h
  filter/IntegerActions.h
  filter/ResourceActions.h
  filter/StringActions.h
  update/AttributeUpdateFactory.h
  update/ItemUpdateFactory.h
  update/ResourceUpdateFactory.h
  utility/Queries.h
)

set(attributeSrcs
  ${jsonAttributeSrcs}
  Analyses.cxx
  AssociationRuleManager.cxx
  AssociationRules.cxx
  Attribute.cxx
  ComponentItem.cxx
  ComponentItemDefinition.cxx
  CopyAssignmentOptions.cxx
  DateTimeItem.cxx
  DateTimeItemDefinition.cxx
  Definition.cxx
  DirectoryInfo.cxx
  DirectoryItem.cxx
  DirectoryItemDefinition.cxx
  DoubleItem.cxx
  DoubleItemDefinition.cxx
  Evaluator.cxx
  EvaluatorFactory.cxx
  EvaluatorManager.cxx
  FileItem.cxx
  FileItemDefinition.cxx
  FileSystemItem.cxx
  FileSystemItemDefinition.cxx
  GroupItem.cxx
  GroupItemDefinition.cxx
  InfixExpressionEvaluator.cxx
  IntItem.cxx
  IntItemDefinition.cxx
  Item.cxx
  ItemDefinition.cxx
  ItemDefinitionManager.cxx
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
  SymbolDependencyStorage.cxx
  ValueItem.cxx
  ValueItemDefinition.cxx
  VoidItem.cxx
  VoidItemDefinition.cxx
  utility/Queries.cxx
)

# construct operator inputs
foreach (operator ${attributeOperators})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
  HEADER_OUTPUT headerName)
  list(APPEND attributeSrcs operators/${operator}.cxx)
  list(APPEND attributeHeaders operators/${operator}.h)
  list(APPEND _attributeDependencies ${headerName})
endforeach()
add_custom_target(attributeGenHeaders DEPENDS ${_attributeDependencies})
# set(attributeDependencies ${_attributeDependencies} PARENT_SCOPE)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  list(APPEND attributeHeaders
    PythonRule.h
    )
  list(APPEND attributeSrcs
    PythonRule.cxx
    )
endif()

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
endif()

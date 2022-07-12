# set up sources to build
set(viewClasses
  AssociationBadge
  AvailableOperations
  BaseView
  BadgeSet
  ComponentPhraseContent
  ComponentPhraseModel
  DescriptivePhrase
  EmptySubphraseGenerator
  Information
  LockedResourceBadge
  Manager
  NameManager
  NodeGroupPhraseContent
  ObjectGroupPhraseContent
  ObjectIconBadge
  ObjectIcons
  OperationDecorator
  OperationIcons
  PhraseListContent
  PhraseModel
  PhraseModelFactory
  QueryFilterSubphraseGenerator
  ReferenceItemPhraseModel
  Registrar
  ResourcePhraseContent
  ResourcePhraseModel
  Selection
  SelectionPhraseModel
  SubphraseGenerator
  SVGIconConstructor
  TwoLevelSubphraseGenerator
  Configuration

  # Serialization
  json/jsonView
  json/jsonNameManager
  xml/xmlConfiguration
)

set(viewSrcs)

set(viewHeaders
  Badge.h
  BadgeFactory.h
  PhraseContent.h
  PhraseModelObserver.h
  SelectionAction.h
  SelectionObserver.h
  SubphraseGeneratorFactory.h
  SubphraseGenerator.txx
  UIElementState.h
  ViewWidgetFactory.h
)

foreach(class ${viewClasses})
  list(APPEND viewHeaders ${class}.h)
  list(APPEND viewSrcs ${class}.cxx)
endforeach()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

set(iconFiles
  icons/attribute.svg
  icons/attributeResource.svg
  icons/checkbox_blanked.svg
  icons/checkbox_partial.svg
  icons/checkbox_visible.svg
  icons/default_operation_opt.svg
  icons/edge.svg
  icons/face.svg
  icons/lock_locked.svg
  icons/loop.svg
  icons/mesh.svg
  icons/meshResource.svg
  icons/model.svg
  icons/modelResource.svg
  icons/resource.svg
  icons/selected.svg
  icons/tree_blanked.svg
  icons/tree_partial.svg
  icons/tree_visible.svg
  icons/vertex.svg
  icons/volume.svg
  icons/unselected.svg
  icons/mode_connection.svg
  icons/mode_pan.svg
  icons/mode_selection.svg
)

set(_viewDependencies)
foreach (iconFile ${iconFiles})
  get_filename_component(genFileBase "${iconFile}" NAME_WE)
  smtk_encode_file(${CMAKE_CURRENT_SOURCE_DIR}/${iconFile}
    TYPE "_cpp" NAME "${genFileBase}_svg"
    HEADER_OUTPUT genFile
    TARGET_OUTPUT genTarget
  )
  list(APPEND viewHeaders ${genFile})
  list(APPEND _viewDependencies ${genTarget})
endforeach()
add_custom_target(viewGenHeaders DEPENDS ${_viewDependencies})
set(viewDependencies ${_viewDependencies} PARENT_SCOPE)

#install the headers
smtk_public_headers(smtkCore ${viewHeaders})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

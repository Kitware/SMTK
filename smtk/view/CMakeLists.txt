# set up sources to build
set(viewSrcs
  AvailableOperations.cxx
  ComponentPhraseContent.cxx
  ComponentPhraseModel.cxx
  DescriptivePhrase.cxx
  EmptySubphraseGenerator.cxx
  json/jsonView.cxx
  Manager.cxx
  ObjectGroupPhraseContent.cxx
  PhraseListContent.cxx
  PhraseModel.cxx
  QueryFilterSubphraseGenerator.cxx
  Registrar.cxx
  ResourcePhraseContent.cxx
  ResourcePhraseModel.cxx
  Selection.cxx
  SelectionPhraseModel.cxx
  SubphraseGenerator.cxx
  TwoLevelSubphraseGenerator.cxx
  Configuration.cxx
  VisibilityContent.cxx
)

set (jsonViewHeaders
  json/jsonView.h
  )
set(viewHeaders
  AvailableOperations.h
  ComponentPhraseContent.h
  ComponentPhraseModel.h
  DescriptivePhrase.h
  EmptySubphraseGenerator.h
  Manager.h
  ObjectGroupPhraseContent.h
  PhraseListContent.h
  PhraseContent.h
  PhraseModel.h
  PhraseModelObserver.h
  QueryFilterSubphraseGenerator.h
  Registrar.h
  ResourcePhraseContent.h
  ResourcePhraseModel.h
  Selection.h
  SelectionAction.h
  SelectionObserver.h
  SelectionPhraseModel.h
  SubphraseGenerator.h
  SubphraseGenerator.txx
  TwoLevelSubphraseGenerator.h
  Configuration.h
  VisibilityContent.h
  ${jsonViewHeaders}
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

#install the headers
smtk_public_headers(smtkCore ${viewHeaders})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

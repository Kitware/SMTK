# set up sources to build
set(viewSrcs
  AvailableOperations.cxx
  ComponentPhraseContent.cxx
  ComponentPhraseModel.cxx
  #ComponentsByTypeGenerator.cxx
  DescriptivePhrase.cxx
  EmptySubphraseGenerator.cxx
  json/jsonView.cxx
  PhraseListContent.cxx
  PhraseModel.cxx
  ResourcePhraseContent.cxx
  ResourcePhraseModel.cxx
  Selection.cxx
  SubphraseGenerator.cxx
  TwoLevelSubphraseGenerator.cxx
  View.cxx
  VisibilityContent.cxx
)

set (jsonViewHeaders
  json/jsonView.h
  )
set(viewHeaders
  AvailableOperations.h
  ComponentPhraseContent.h
  ComponentPhraseModel.h
  #ComponentsByTypeGenerator.h
  DescriptivePhrase.h
  EmptySubphraseGenerator.h
  PhraseListContent.h
  PhraseContent.h
  PhraseModel.h
  ResourcePhraseContent.h
  ResourcePhraseModel.h
  Selection.h
  SubphraseGenerator.h
  SubphraseGenerator.txx
  TwoLevelSubphraseGenerator.h
  View.h
  VisibilityContent.h
  ${jsonViewHeaders}
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

#install the headers
smtk_public_headers(smtkCore ${viewHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

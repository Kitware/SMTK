# set up sources to build
set(viewSrcs
  ComponentPhraseContent.cxx
  #ComponentsByTypeGenerator.cxx
  DescriptivePhrase.cxx
  json/jsonView.cxx
  PhraseListContent.cxx
  PhraseModel.cxx
  ResourcePhraseContent.cxx
  ResourcePhraseModel.cxx
  Selection.cxx
  SubphraseGenerator.cxx
  View.cxx
  VisibilityContent.cxx
)

set (jsonViewHeaders
  json/jsonView.h
  )
set(viewHeaders
  ComponentPhraseContent.h
  #ComponentsByTypeGenerator.h
  DescriptivePhrase.h
  PhraseListContent.h
  PhraseContent.h
  PhraseModel.h
  ResourcePhraseContent.h
  ResourcePhraseModel.h
  Selection.h
  SubphraseGenerator.h
  SubphraseGenerator.txx
  View.h
  VisibilityContent.h
  ${jsonViewHeaders}
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

#install the headers
smtk_public_headers(${viewHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
  target_link_libraries(TestBuild_smtk_view
    PUBLIC nlohmann_json)
endif()

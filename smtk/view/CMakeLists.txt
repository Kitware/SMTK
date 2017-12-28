# set up sources to build
set(viewSrcs
  ComponentPhrase.cxx
  DescriptivePhrase.cxx
  json/jsonView.cxx
  PhraseList.cxx
  PhraseModel.cxx
  ResourcePhrase.cxx
  ResourcePhraseModel.cxx
  Selection.cxx
  SubphraseGenerator.cxx
  View.cxx
)

set (jsonViewHeaders
  json/jsonView.h
  )
set(viewHeaders
  ComponentPhrase.h
  DescriptivePhrase.h
  PhraseList.h
  PhraseList.txx
  PhraseModel.h
  ResourcePhrase.h
  ResourcePhraseModel.h
  Selection.h
  SubphraseGenerator.h
  SubphraseGenerator.txx
  View.h
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

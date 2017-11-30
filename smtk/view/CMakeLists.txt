# set up sources to build
set(viewSrcs
  ComponentPhrase.cxx
  DescriptivePhrase.cxx
  PhraseList.cxx
  PhraseModel.cxx
  ResourcePhrase.cxx
  ResourcePhraseModel.cxx
  SubphraseGenerator.cxx
  View.cxx
)

set(viewHeaders
  ComponentPhrase.h
  DescriptivePhrase.h
  PhraseList.h
  PhraseList.txx
  PhraseModel.h
  ResourcePhrase.h
  ResourcePhraseModel.h
  SubphraseGenerator.h
  SubphraseGenerator.txx
  View.h
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

#install the headers
smtk_public_headers(${viewHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

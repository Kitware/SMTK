set(stringSrcs
  Manager.cxx
  Token.cxx
  json/jsonManager.cxx
  json/jsonToken.cxx
)

set(stringHeaders
  Manager.h
  Token.h
  json/jsonManager.h
  json/jsonToken.h
)

#install the headers
smtk_public_headers(smtkCore ${stringHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

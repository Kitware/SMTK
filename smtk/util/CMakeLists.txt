project(SMTK_Util)

# set up sources to build
set(utilSrcs
  AttributeReader.cxx
  AttributeWriter.cxx
  Logger.cxx
  UserData.cxx
  UUID.cxx
  XmlDocV1Parser.cxx
  XmlV1StringWriter.cxx
  )

set(utilHeaders
  AttributeReader.h
  AttributeWriter.h
  Logger.h
  UserData.h
  UUID.h
  XmlDocV1Parser.h
  XmlV1StringWriter.h
  SystemConfig.h
  )

#Install headers
smtk_public_headers(${utilHeaders})

if(SMTK_ENABLE_TESTING)
  add_subdirectory(Testing)
endif(SMTK_ENABLE_TESTING)

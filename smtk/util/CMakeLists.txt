project(SMTK_Util)

# set up sources to build
set(utilSrcs
  AttributeReader.cxx
  AttributeWriter.cxx
  ExportSpec.cxx
  Logger.cxx
  Resource.cxx
  ResourceSet.cxx
  ResourceSetReader.cxx
  ResourceSetWriter.cxx
  UserData.cxx
  UUID.cxx
  UUIDGenerator.cxx
  XmlDocV2Parser.cxx
  XmlV2StringWriter.cxx
  )

set(utilHeaders
  AttributeReader.h
  AttributeWriter.h
  AutoInit.h
  ExportSpec.h
  Logger.h
  Resource.h
  ResourceSet.h
  ResourceSetReader.h
  ResourceSetWriter.h
  SharedFromThis.h
  UserData.h
  UUID.h
  UUIDGenerator.h
  XmlDocV2Parser.h
  XmlV2StringWriter.h
  SystemConfig.h
  )

#Install headers
smtk_public_headers(${utilHeaders})

if(SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif(SMTK_ENABLE_TESTING)

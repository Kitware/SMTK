# set up sources to build
set(ioSrcs
  AttributeReader.cxx
  AttributeWriter.cxx
  ExportJSON.cxx
  ImportJSON.cxx
  Logger.cxx
  ResourceSetReader.cxx
  ResourceSetWriter.cxx
  XmlDocV1Parser.cxx
  XmlDocV2Parser.cxx
  XmlV2StringWriter.cxx
)

set(ioHeaders
  AttributeReader.h
  AttributeWriter.h
  ExportJSON.h
  ImportJSON.h
  Logger.h
  ResourceSetReader.h
  ResourceSetWriter.h
  #XmlDocV1Parser.h
  XmlDocV2Parser.h
  #XmlV1StringWriter.h
  XmlV2StringWriter.h
)

#install the headers
smtk_public_headers(${ioHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

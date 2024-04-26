# set up sources to build
set(ioSrcs
  attributeUtils.cxx
  AttributeReader.cxx
  AttributeWriter.cxx
  Helpers.cxx
  json/jsonComponentSet.cxx
  json/jsonSelectionMap.cxx
  Logger.cxx
  ModelToMesh.cxx
  TemplateInfo.cxx
  XmlDocV1Parser.cxx
  XmlDocV2Parser.cxx
  XmlDocV3Parser.cxx
  XmlDocV4Parser.cxx
  XmlDocV5Parser.cxx
  XmlDocV6Parser.cxx
  XmlDocV7Parser.cxx
  XmlDocV8Parser.cxx
  XmlV2StringWriter.cxx
  XmlV3StringWriter.cxx
  XmlV4StringWriter.cxx
  XmlV5StringWriter.cxx
  XmlV6StringWriter.cxx
  XmlV7StringWriter.cxx
  XmlV8StringWriter.cxx
)

set(ioHeaders
  attributeUtils.h
  AttributeReader.h
  AttributeWriter.h
  Helpers.h
  json/jsonComponentSet.h
  json/jsonSelectionMap.h
  Logger.h
  ModelToMesh.h
  TemplateInfo.h
  XmlDocV1Parser.h
  XmlDocV2Parser.h
  XmlDocV3Parser.h
  XmlDocV4Parser.h
  XmlDocV5Parser.h
  XmlDocV6Parser.h
  XmlDocV7Parser.h
  XmlDocV8Parser.h
  XmlStringWriter.h
  XmlV2StringWriter.h
  XmlV3StringWriter.h
  XmlV4StringWriter.h
  XmlV5StringWriter.h
  XmlV6StringWriter.h
  XmlV7StringWriter.h
  Xml87StringWriter.h
)

# mesh-related I/O
list(APPEND ioSrcs
  ExportMesh.cxx
  ImportMesh.cxx
  ReadMesh.cxx
  WriteMesh.cxx
  mesh/Format.cxx
  mesh/MeshIOMoab.cxx
  mesh/MeshIOXMS.cxx
)

list(APPEND ioHeaders
  ExportMesh.h
  ImportMesh.h
  ReadMesh.h
  WriteMesh.h
  mesh/Format.h
  mesh/MeshIO.h
  mesh/MeshIOMoab.h
  mesh/MeshIOXMS.h
)

#install the headers
smtk_public_headers(smtkCore ${ioHeaders})

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

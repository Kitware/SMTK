# set up sources to build
set(ioSrcs
  AttributeReader.cxx
  AttributeWriter.cxx
  ExportJSON.cxx
  ImportJSON.cxx
  ImportMesh.cxx
  Logger.cxx
  ModelToMesh.cxx
  MeshExport2DM3DM.cxx
  OperatorLog.cxx
  ResourceSetReader.cxx
  ResourceSetWriter.cxx
  WriteMesh.cxx
  XmlDocV1Parser.cxx
  XmlDocV2Parser.cxx
  XmlV2StringWriter.cxx
)

set(ioHeaders
  AttributeReader.h
  AttributeWriter.h
  ExportJSON.h
  ImportJSON.h
  ImportMesh.h
  Logger.h
  ModelToMesh.h
  MeshExport2DM.h
  MeshExport3DM.h
  OperatorLog.h
  ResourceSetReader.h
  ResourceSetWriter.h
  WriteMesh.h
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

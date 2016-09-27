# set up sources to build
set(ioSrcs
  AttributeReader.cxx
  AttributeWriter.cxx
  ExportJSON.cxx
  ImportJSON.cxx
  Logger.cxx
  OperatorLog.cxx
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
  OperatorLog.h
  ResourceSetReader.h
  ResourceSetWriter.h
  #XmlDocV1Parser.h
  XmlDocV2Parser.h
  XmlV2StringWriter.h
)

# mesh-related I/O
list(APPEND ioSrcs
  mesh/ImportMesh.cxx
  mesh/MeshExport2DM3DM.cxx
  mesh/ModelToMesh.cxx
  mesh/WriteMesh.cxx
)

list(APPEND ioHeaders
  mesh/ImportMesh.h
  mesh/MeshExport2DM.h
  mesh/MeshExport3DM.h
  mesh/ModelToMesh.h
  mesh/WriteMesh.h
)

#install the headers
smtk_public_headers(${ioHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

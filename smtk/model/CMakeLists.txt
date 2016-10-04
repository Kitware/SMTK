# set up sources to build
set(modelSrcs
  Arrangement.cxx
  ArrangementHelper.cxx
  ArrangementKind.cxx
  AttributeAssignments.cxx
  AttributeListPhrase.cxx
  AuxiliaryGeometry.cxx
  Session.cxx
  SessionRef.cxx
  SessionIOJSON.cxx
  SessionRegistrar.cxx
  CellEntity.cxx
  Chain.cxx
  EntityRef.cxx
  EntityRefArrangementOps.cxx
  DefaultSession.cxx
  DescriptivePhrase.cxx
  Edge.cxx
  EdgeUse.cxx
  Entity.cxx
  EntityIterator.cxx
  EntityListPhrase.cxx
  EntityPhrase.cxx
  EntityTypeSubphrases.cxx
  Face.cxx
  FaceUse.cxx
  Group.cxx
  GridInfo.cxx
  Instance.cxx
  Loop.cxx
  MeshListPhrase.cxx
  MeshPhrase.cxx
  Model.cxx
  Operator.cxx
  PropertyListPhrase.cxx
  PropertyValuePhrase.cxx
  RemoteOperator.cxx
  Shell.cxx
  ShellEntity.cxx
  SimpleModelSubphrases.cxx
  Manager.cxx
  SubphraseGenerator.cxx
  Tessellation.cxx
  UseEntity.cxx
  Vertex.cxx
  VertexUse.cxx
  Volume.cxx
  VolumeUse.cxx
  operators/CloseModel.cxx
  operators/SetProperty.cxx
  operators/ExportModelJSON.cxx
  operators/ExportSMTKModel.cxx
  operators/ImportSMTKModel.cxx
  operators/DeleteMesh.cxx
  operators/ExportMesh.cxx
  operators/WriteMesh.cxx
  )

set(modelHeaders
  Arrangement.h
  ArrangementHelper.h
  ArrangementKind.h
  AttributeAssignments.h
  AttributeListPhrase.h
  AuxiliaryGeometry.h
  Session.h
  SessionRef.h
  SessionIO.h
  SessionIOJSON.h
  SessionRegistrar.h
  CellEntity.h
  Chain.h
  EntityRef.h
  EntityRefArrangementOps.h
  DefaultSession.h
  DescriptivePhrase.h
  Edge.h
  EdgeUse.h
  Entity.h
  EntityIterator.h
  EntityListPhrase.h
  EntityPhrase.h
  EntityTypeBits.h
  EntityTypeSubphrases.h
  Events.h
  Face.h
  FaceUse.h
  FloatData.h
  GridInfo.h
  Group.h
  Instance.h
  IntegerData.h
  Loop.h
  MeshListPhrase.h
  MeshPhrase.h
  Model.h
  Operator.h
  PropertyType.h
  PropertyListPhrase.h
  PropertyValuePhrase.h
  RemoteOperator.h
  Shell.h
  ShellEntity.h
  SimpleModelSubphrases.h
  Manager.h
  SubphraseGenerator.h
  StringData.h
  Tessellation.h
  UseEntity.h
  Vertex.h
  VertexUse.h
  Volume.h
  VolumeUse.h
  operators/CloseModel.h
  operators/SetProperty.h
  operators/ExportModelJSON.h
  operators/ExportSMTKModel.h
  operators/ImportSMTKModel.h
  operators/DeleteMesh.h
  operators/ExportMesh.h
  operators/WriteMesh.h
  )

smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/DefaultSession.json" defSessionJSON)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/CloseModel.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SetProperty.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ExportModelJSON.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ExportSMTKModel.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ImportSMTKModel.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/DeleteMesh.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ExportMesh.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/WriteMesh.sbt" defOpXML)

#install the headers
smtk_public_headers(${modelHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

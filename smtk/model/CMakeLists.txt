# set up sources to build
set(modelOps
  AddAuxiliaryGeometry
  AddImage
  AssignColors
  CloseModel
  CompositeAuxiliaryGeometry
  CreateInstances
  EntityGroupOperator
  ExportModelJSON
  ExportSMTKModel
  GroupAuxiliaryGeometry
  LoadSMTKModel
  SaveSMTKModel
  SetProperty
  TerrainExtraction
)

set(modelSrcs
  Arrangement.cxx
  ArrangementHelper.cxx
  ArrangementKind.cxx
  AttributeAssignments.cxx
  AuxiliaryGeometry.cxx
  AuxiliaryGeometryExtension.cxx
  Session.cxx
  SessionRef.cxx
  SessionIO.cxx
  SessionIOJSON.cxx
  SessionRegistrar.cxx
  CellEntity.cxx
  Chain.cxx
  EntityRef.cxx
  EntityRefArrangementOps.cxx
  DefaultSession.cxx
  Edge.cxx
  EdgeUse.cxx
  Entity.cxx
  EntityIterator.cxx
  Face.cxx
  FaceUse.cxx
  Group.cxx
  GridInfo.cxx
  Instance.cxx
  Loop.cxx
  Model.cxx
  PointLocatorExtension.cxx
  RegisterResources.cxx
  RegisterOperations.cxx
  Shell.cxx
  ShellEntity.cxx
  Manager.cxx
  Tessellation.cxx
  UseEntity.cxx
  Vertex.cxx
  VertexUse.cxx
  Volume.cxx
  VolumeUse.cxx
  json/jsonArrangement.cxx
  json/jsonEntity.cxx
  json/jsonEntityIterator.cxx
  json/jsonResource.cxx
  json/jsonTessellation.cxx
)

set(modelHeaders
  Arrangement.h
  ArrangementHelper.h
  ArrangementKind.h
  AttributeAssignments.h
  AuxiliaryGeometry.h
  AuxiliaryGeometryExtension.h
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
  Edge.h
  EdgeUse.h
  Entity.h
  EntityIterator.h
  EntityTypeBits.h
  Events.h
  Face.h
  FaceUse.h
  FloatData.h
  GridInfo.h
  Group.h
  Instance.h
  IntegerData.h
  Loop.h
  Model.h
  PointLocatorExtension.h
  RegisterResources.h
  RegisterOperations.h
  Shell.h
  ShellEntity.h
  Manager.h
  StringData.h
  Tessellation.h
  UseEntity.h
  Vertex.h
  VertexUse.h
  Volume.h
  VolumeUse.h
  json/jsonArrangement.h
  json/jsonEntity.h
  json/jsonEntityIterator.h
  json/jsonResource.h
  json/jsonTessellation.h
)

smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/DefaultSession.json" defSessionJSON)
foreach (operator ${modelOps})
  smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt" defOpXML)
  list(APPEND modelSrcs operators/${operator}.cxx)
  list(APPEND modelHeaders operators/${operator}.h)
endforeach()

#install the headers
smtk_public_headers(${modelHeaders})

add_subdirectory(environment)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_model
    PUBLIC
    nlohmann_json)
  add_subdirectory(testing)
endif()

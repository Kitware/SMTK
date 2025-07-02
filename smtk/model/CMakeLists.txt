# set up sources to build
set(modelOps
  AddAuxiliaryGeometry
  AddImage
  CloseModel
  CompositeAuxiliaryGeometry
  CreateInstances
  Delete
  DivideInstance
  EntityGroupOperation
  ExportModelJSON
  GroupAuxiliaryGeometry
  MergeInstances
  SetInstancePrototype
  TerrainExtraction
)

set(modelSrcs
  Arrangement.cxx
  ArrangementHelper.cxx
  ArrangementKind.cxx
  AttributeAssignments.cxx
  AuxiliaryGeometry.cxx
  AuxiliaryGeometryExtension.cxx
  CellEntity.cxx
  Chain.cxx
  DefaultSession.cxx
  EntityRef.cxx
  EntityRefArrangementOps.cxx
  Edge.cxx
  EdgeUse.cxx
  Entity.cxx
  EntityIterator.cxx
  Face.cxx
  FaceUse.cxx
  Group.cxx
  Instance.cxx
  Loop.cxx
  Model.cxx
  PointLocatorExtension.cxx
  Registrar.cxx
  Session.cxx
  SessionRef.cxx
  SessionIO.cxx
  SessionIOJSON.cxx
  Shell.cxx
  ShellEntity.cxx
  Resource.cxx
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
  utility/InterpolateField.cxx
)

set(modelHeaders
  Arrangement.h
  Arrangement.txx
  ArrangementHelper.h
  ArrangementKind.h
  AttributeAssignments.h
  AuxiliaryGeometry.h
  AuxiliaryGeometryExtension.h
  Session.h
  SessionRef.h
  SessionIO.h
  SessionIOJSON.h
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
  FilterGrammar.h
  FloatData.h
  Group.h
  Instance.h
  IntegerData.h
  LimitingClause.h
  Loop.h
  Model.h
  PointLocatorExtension.h
  Registrar.h
  Shell.h
  ShellEntity.h
  ShellEntity.txx
  Resource.h
  Resource.txx
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
  utility/InterpolateField.h
)

set(icons)
foreach(icon ${icons})
  smtk_encode_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/icons/${icon}.svg" TYPE "_svg" TARGET_OUTPUT targetName)
  list(APPEND smtkModelDependencies ${targetName})
endforeach()

foreach (operator ${modelOps})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
    HEADER_OUTPUT headerName)
  list(APPEND modelSrcs operators/${operator}.cxx)
  list(APPEND modelHeaders operators/${operator}.h)
  list(APPEND _modelDependencies ${headerName})
endforeach()
add_custom_target(modelGenHeaders
  DEPENDS
    ${_modelDependencies}
    ${smtkModelDependencies}
)
# set(modelDependencies ${_modelDependencies} PARENT_SCOPE)

#install the headers
smtk_public_headers(smtkCore ${modelHeaders})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

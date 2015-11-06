# set up sources to build
set(meshSrcs
  CellSet.cxx
  CellTypes.cxx
  Collection.cxx
  ExtractTessellation.cxx
  ForEachTypes.cxx
  Handle.cxx
  Manager.cxx
  MeshSet.cxx
  PointConnectivity.cxx
  PointSet.cxx
  TypeSet.cxx

  json/Interface.cxx
  json/MeshInfo.cxx
  json/Readers.cxx

  moab/Allocator.cxx
  moab/CellTypeToType.cxx
  moab/Interface.cxx
  moab/ConnectivityStorage.cxx
  moab/MergeMeshVertices.cxx
  moab/Readers.cxx
  moab/Writers.cxx

  operators/SetProperty.cxx
  )

set(meshHeaders
  CellSet.h
  CellTraits.h
  CellTypes.h
  Collection.h
  DimensionTypes.h
  ExtractTessellation.h
  ForEachTypes.h
  Handle.h
  Interface.h
  Manager.h
  MeshSet.h
  PointConnectivity.h
  PointSet.h
  PropertyData.h
  QueryTypes.h
  TypeSet.h

  #Limit the amount of headers for each backend we install. These should be
  #implementation details users of smtk don't get access to ( outside the interface )
  json/Interface.h
  json/MeshInfo.h
  moab/Interface.h
  moab/Allocator.h
  moab/HandleRange.h

  operators/SetProperty.h
  )

smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SetProperty.sbt" defOpXML)

#install the headers
smtk_public_headers(${meshHeaders})

if(SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

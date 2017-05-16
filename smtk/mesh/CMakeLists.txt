# set up sources to build
set(meshSrcs
  CellSet.cxx
  CellField.cxx
  CellTypes.cxx
  Collection.cxx
  Displace.cxx
  ExtractMeshConstants.cxx
  ExtractTessellation.cxx
  ForEachTypes.cxx
  Handle.cxx
  Manager.cxx
  MeshSet.cxx
  PointConnectivity.cxx
  PointField.cxx
  PointLocator.cxx
  PointSet.cxx
  QueryTypes.cxx
  Reclassify.cxx
  TypeSet.cxx

  json/Interface.cxx
  json/MeshInfo.cxx
  json/Readers.cxx

  moab/Allocator.cxx
  moab/CellTypeToType.cxx
  moab/BufferedCellAllocator.cxx
  moab/IncrementalAllocator.cxx
  moab/Interface.cxx
  moab/ConnectivityStorage.cxx
  moab/MergeMeshVertices.cxx
  moab/PointLocatorImpl.cxx
  moab/Readers.cxx
  moab/Writers.cxx
  )

set(meshHeaders
  CellSet.h
  CellField.h
  CellTraits.h
  CellTypes.h
  Collection.h
  DimensionTypes.h
  Displace.h
  ExtractMeshConstants.h
  ExtractTessellation.h
  ForEachTypes.h
  Handle.h
  Interface.h
  Manager.h
  MeshSet.h
  PointConnectivity.h
  PointField.h
  PointSet.h
  PropertyData.h
  QueryTypes.h
  Reclassify.h
  TypeSet.h

  #Limit the amount of headers for each backend we install. These should be
  #implementation details users of smtk don't get access to ( outside the interface )
  json/Interface.h
  json/MeshInfo.h
  moab/Interface.h
  moab/HandleRange.h
  )

#install the headers
smtk_public_headers(${meshHeaders})

if (SMTK_USE_PYBIND11)
  add_subdirectory(pybind11)
endif()

if(SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

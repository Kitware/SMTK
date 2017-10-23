# set up sources to build
set(meshSrcs
  ApplyToMesh.cxx
  CellSet.cxx
  CellField.cxx
  CellTypes.cxx
  Collection.cxx
  ExtractMeshConstants.cxx
  ExtractTessellation.cxx
  ForEachTypes.cxx
  Handle.cxx
  Manager.cxx
  MeshSet.cxx
  Metrics.cxx
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

  interpolation/InverseDistanceWeighting.cxx
  interpolation/RadialAverage.cxx

  operators/DeleteMesh.cxx
  operators/ExportMesh.cxx
  operators/GenerateHotStartData.cxx
  operators/InterpolateOntoMesh.cxx
  operators/UndoWarpMesh.cxx
  operators/WarpMesh.cxx
  operators/WriteMesh.cxx
  )

set(meshHeaders
  ApplyToMesh.h
  CellSet.h
  CellField.h
  CellTraits.h
  CellTypes.h
  Collection.h
  DimensionTypes.h
  ExtractMeshConstants.h
  ExtractTessellation.h
  ForEachTypes.h
  Handle.h
  Interface.h
  Manager.h
  MeshSet.h
  Metrics.h
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

  interpolation/InverseDistanceWeighting.h
  interpolation/PointCloud.h
  interpolation/RadialAverage.h
  interpolation/StructuredGrid.h

  operators/DeleteMesh.h
  operators/ExportMesh.h
  operators/GenerateHotStartData.h
  operators/InterpolateOntoMesh.h
  operators/UndoWarpMesh.h
  operators/WarpMesh.h
  operators/WriteMesh.h
  )

smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/DeleteMesh.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ExportMesh.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/GenerateHotStartData.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/InterpolateOntoMesh.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/UndoWarpMesh.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/WarpMesh.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/WriteMesh.sbt" defOpXML)

#install the headers
smtk_public_headers(${meshHeaders})

if (SMTK_USE_PYBIND11)
  add_subdirectory(pybind11)
endif()

if(SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

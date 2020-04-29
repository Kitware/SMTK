# set up sources to build
set(meshSrcs
  core/CellSet.cxx
  core/CellField.cxx
  core/CellTypes.cxx
  core/Resource.cxx
  core/Component.cxx
  core/ForEachTypes.cxx
  core/Handle.cxx
  core/MeshSet.cxx
  core/PointConnectivity.cxx
  core/PointField.cxx
  core/PointLocator.cxx
  core/PointSet.cxx
  core/QueryTypes.cxx
  core/TypeSet.cxx

  interpolation/InverseDistanceWeighting.cxx
  interpolation/PointCloudFromCSV.cxx
  interpolation/PointCloudGenerator.cxx
  interpolation/RadialAverage.cxx
  interpolation/StructuredGridGenerator.cxx

  json/Interface.cxx
  json/MeshInfo.cxx
  json/jsonHandleRange.cxx
  json/jsonMeshInfo.cxx
  json/jsonResource.cxx

  moab/Allocator.cxx
  moab/BufferedCellAllocator.cxx
  moab/CellTypeToType.cxx
  moab/ClosestPoint.cxx
  moab/DistanceTo.cxx
  moab/HandleRangeToRange.cxx
  moab/IncrementalAllocator.cxx
  moab/Interface.cxx
  moab/ConnectivityStorage.cxx
  moab/MergeMeshVertices.cxx
  moab/ModelEntityPointLocator.cxx
  moab/PointLocatorCache.cxx
  moab/PointLocatorImpl.cxx
  moab/RandomPoint.cxx
  moab/Readers.cxx
  moab/Writers.cxx

  resource/Registrar.cxx
  resource/Selection.cxx

  utility/ApplyToMesh.cxx
  utility/Create.cxx
  utility/ExtractCanonicalIndices.cxx
  utility/ExtractMeshConstants.cxx
  utility/ExtractTessellation.cxx
  utility/Metrics.cxx
  utility/Reclassify.cxx
  )

set(meshHeaders
  core/CellSet.h
  core/CellField.h
  core/CellTraits.h
  core/CellTypes.h
  core/Resource.h
  core/Component.h
  core/DimensionTypes.h
  core/FieldTypes.h
  core/ForEachTypes.h
  core/Handle.h
  core/Interface.h
  core/MeshSet.h
  core/PointConnectivity.h
  core/PointField.h
  core/PointSet.h
  core/QueryTypes.h
  core/TypeSet.h

  core/queries/BoundingBox.h

  interpolation/InverseDistanceWeighting.h
  interpolation/PointCloud.h
  interpolation/PointCloudFromCSV.h
  interpolation/PointCloudGenerator.h
  interpolation/RadialAverage.h
  interpolation/StructuredGrid.h
  interpolation/StructuredGridGenerator.h

  #Limit the amount of headers for each backend we install. These should be
  #implementation details users of smtk don't get access to ( outside the interface )
  json/Interface.h
  json/MeshInfo.h
  json/jsonHandleRange.h
  json/jsonMeshInfo.h
  json/jsonResource.h

  moab/HandleRangeToRange.h
  moab/Interface.h
  moab/ModelEntityPointLocator.h

  resource/Registrar.h
  resource/Selection.h

  utility/ApplyToMesh.h
  utility/Create.h
  utility/ExtractCanonicalIndices.h
  utility/ExtractMeshConstants.h
  utility/ExtractTessellation.h
  utility/Metrics.h
  utility/Reclassify.h
  )
set(meshOperators
  DeleteMesh
  ElevateMesh
  Export
  ExtractAdjacency
  ExtractByDihedralAngle
  ExtractSkin
  GenerateHotStartData
  Import
  InterpolateOntoMesh
  MergeCoincidentPoints
  PrintMeshInformation
  Read
  ReadResource
  SelectCells
  SetMeshName
  Subtract
  Transform
  UndoElevateMesh
  Write
  WriteResource
  )

# construct operator inputs
foreach (operator ${meshOperators})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
  HEADER_OUTPUT headerName)
  list(APPEND meshSrcs operators/${operator}.cxx)
  list(APPEND meshHeaders operators/${operator}.h)
  list(APPEND _meshDependencies ${headerName})
endforeach()
add_custom_target(meshGenHeaders DEPENDS ${_meshDependencies})
set(meshDependencies ${_meshDependencies} PARENT_SCOPE)

#install the headers
smtk_public_headers(smtkCore ${meshHeaders})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if(SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

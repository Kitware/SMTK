# set up sources to build
set(meshSrcs
  core/CellSet.cxx
  core/CellField.cxx
  core/CellTypes.cxx
  core/Collection.cxx
  core/ForEachTypes.cxx
  core/Handle.cxx
  core/Manager.cxx
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
  json/Readers.cxx
  json/jsonHandleRange.cxx

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

  operators/DeleteMesh.cxx
  operators/ElevateMesh.cxx
  operators/ExportMesh.cxx
  operators/GenerateHotStartData.cxx
  operators/InterpolateOntoMesh.cxx
  operators/UndoElevateMesh.cxx
  operators/WriteMesh.cxx

  resource/MeshComponent.cxx
  resource/Registrar.cxx

  utility/ApplyToMesh.cxx
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
  core/Collection.h
  core/DimensionTypes.h
  core/FieldTypes.h
  core/ForEachTypes.h
  core/Handle.h
  core/Interface.h
  core/Manager.h
  core/MeshSet.h
  core/PointConnectivity.h
  core/PointField.h
  core/PointSet.h
  core/QueryTypes.h
  core/TypeSet.h

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

  moab/Interface.h
  moab/HandleRange.h

  operators/DeleteMesh.h
  operators/ElevateMesh.h
  operators/ExportMesh.h
  operators/GenerateHotStartData.h
  operators/InterpolateOntoMesh.h
  operators/UndoElevateMesh.h
  operators/WriteMesh.h

  resource/MeshComponent.h
  resource/PropertyData.h
  resource/Registrar.h

  utility/ApplyToMesh.h
  utility/ExtractMeshConstants.h
  utility/ExtractTessellation.h
  utility/Metrics.h
  utility/Reclassify.h
  )

smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/DeleteMesh.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ElevateMesh.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ExportMesh.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/GenerateHotStartData.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/InterpolateOntoMesh.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/UndoElevateMesh.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/WriteMesh.sbt" defOpXML)

#install the headers
smtk_public_headers(${meshHeaders})

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if(SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_mesh
    PUBLIC
    nlohmann_json)
  add_subdirectory(testing)
endif()

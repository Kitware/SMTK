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
  moab/CellTypeToType.cxx
  moab/BufferedCellAllocator.cxx
  moab/HandleRangeToRange.cxx
  moab/IncrementalAllocator.cxx
  moab/Interface.cxx
  moab/ConnectivityStorage.cxx
  moab/MergeMeshVertices.cxx
  moab/ModelEntityPointLocator.cxx
  moab/PointLocatorImpl.cxx
  moab/Readers.cxx
  moab/Writers.cxx

  operators/DeleteMesh.cxx
  operators/ElevateMesh.cxx
  operators/Export.cxx
  operators/ExtractAdjacency.cxx
  operators/ExtractByDihedralAngle.cxx
  operators/ExtractSkin.cxx
  operators/GenerateHotStartData.cxx
  operators/Import.cxx
  operators/InterpolateOntoMesh.cxx
  operators/MergeCoincidentPoints.cxx
  operators/PrintMeshInformation.cxx
  operators/Read.cxx
  operators/ReadResource.cxx
  operators/SelectCells.cxx
  operators/SetMeshName.cxx
  operators/UndoElevateMesh.cxx
  operators/Write.cxx
  operators/WriteResource.cxx

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

  operators/DeleteMesh.h
  operators/ElevateMesh.h
  operators/Export.h
  operators/ExtractAdjacency.h
  operators/ExtractByDihedralAngle.h
  operators/ExtractSkin.h
  operators/GenerateHotStartData.h
  operators/Import.h
  operators/InterpolateOntoMesh.h
  operators/MergeCoincidentPoints.h
  operators/PrintMeshInformation.h
  operators/Read.h
  operators/ReadResource.h
  operators/SelectCells.h
  operators/SetMeshName.h
  operators/UndoElevateMesh.h
  operators/Write.h
  operators/WriteResource.h

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

smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/DeleteMesh.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ElevateMesh.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Export.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ExtractAdjacency.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ExtractByDihedralAngle.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ExtractSkin.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/GenerateHotStartData.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Import.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/InterpolateOntoMesh.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/MergeCoincidentPoints.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/PrintMeshInformation.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Read.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ReadResource.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SelectCells.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SetMeshName.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/UndoElevateMesh.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Write.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/WriteResource.sbt" defOpXML)

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

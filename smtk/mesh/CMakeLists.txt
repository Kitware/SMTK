# set up sources to build
set(meshSrcs
  core/CellSet.cxx
  core/CellField.cxx
  core/CellTypes.cxx
  core/Collection.cxx
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
  json/jsonCollection.cxx

  moab/Allocator.cxx
  moab/CellTypeToType.cxx
  moab/BufferedCellAllocator.cxx
  moab/HandleRangeToRange.cxx
  moab/IncrementalAllocator.cxx
  moab/Interface.cxx
  moab/ConnectivityStorage.cxx
  moab/MergeMeshVertices.cxx
  moab/PointLocatorImpl.cxx
  moab/Readers.cxx
  moab/Writers.cxx

  operators/DeleteMesh.cxx
  operators/ElevateMesh.cxx
  operators/Export.cxx
  operators/GenerateHotStartData.cxx
  operators/Import.cxx
  operators/InterpolateOntoMesh.cxx
  operators/Read.cxx
  operators/SetMeshName.cxx
  operators/UndoElevateMesh.cxx
  operators/Write.cxx

  resource/Registrar.cxx

  utility/ApplyToMesh.cxx
  utility/Create.cxx
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
  json/jsonCollection.h

  moab/Interface.h

  operators/DeleteMesh.h
  operators/ElevateMesh.h
  operators/Export.h
  operators/GenerateHotStartData.h
  operators/Import.h
  operators/InterpolateOntoMesh.h
  operators/Read.h
  operators/SetMeshName.h
  operators/UndoElevateMesh.h
  operators/Write.h

  resource/PropertyData.h
  resource/Registrar.h

  utility/ApplyToMesh.h
  utility/Create.h
  utility/ExtractMeshConstants.h
  utility/ExtractTessellation.h
  utility/Metrics.h
  utility/Reclassify.h
  )

smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/DeleteMesh.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ElevateMesh.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Export.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/GenerateHotStartData.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Import.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/InterpolateOntoMesh.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Read.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SetMeshName.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/UndoElevateMesh.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/Write.sbt" defOpXML)

#install the headers
smtk_public_headers(smtkCore ${meshHeaders})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_smtk_plugin(
    smtkMeshPlugin "1.0"
    REGISTRAR smtk::mesh::Registrar
    REGISTRAR_HEADER smtk/mesh/resource/Registrar.h
    MANAGERS smtk::operation::Manager smtk::resource::Manager
    LIBRARIES smtkCore)
  smtk_install_library(smtkMeshPlugin)
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if(SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

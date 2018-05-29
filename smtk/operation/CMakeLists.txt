set(operationSrcs
  Group.cxx
  Manager.cxx
  Metadata.cxx
  MetadataObserver.cxx
  Observer.cxx
  Operation.cxx
  Registrar.cxx
  ResourceManagerOperation.cxx
  SpecificationOps.cxx
  XMLOperation.cxx

  groups/ImporterGroup.cxx
  groups/ReaderGroup.cxx
  groups/ResourceIOGroup.cxx
  groups/WriterGroup.cxx

  operators/ImportResource.cxx
  operators/ReadResource.cxx
  operators/WriteResource.cxx
)

set(operationHeaders
  Group.h
  Manager.h
  Metadata.h
  MetadataContainer.h
  MetadataObserver.h
  Observer.h
  Operation.h
  Registrar.h
  ResourceManagerOperation.h
  XMLOperation.h

  groups/ImporterGroup.h
  groups/ReaderGroup.h
  groups/ResourceIOGroup.h
  groups/WriterGroup.h

  operators/ImportResource.h
  operators/ReadResource.h
  operators/WriteResource.h
)

smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/Operation.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ReadResource.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/WriteResource.sbt" defOpXML)

if (SMTK_ENABLE_PYTHON_WRAPPING)

  list (APPEND operationSrcs
    RegisterPythonOperations.cxx
    operators/ImportPythonOperation.cxx)
  list (APPEND operationHeaders
    RegisterPythonOperations.h
    operators/ImportPythonOperation.h)

  add_subdirectory(pybind11)

endif()

#install the headers
smtk_public_headers(${operationHeaders})

if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_operation
    PUBLIC nlohmann_json)
  add_subdirectory(testing)
endif()

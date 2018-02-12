set(operationSrcs
  CreateResource.cxx
  LoadResource.cxx
  Manager.cxx
  Metadata.cxx
  MetadataObserver.cxx
  Observer.cxx
  Operation.cxx
  RegisterOperations.cxx
  RegisterResources.cxx
  ResourceManagerOperation.cxx
  SaveResource.cxx
  SpecificationOps.cxx
  XMLOperation.cxx
)

set(operationHeaders
  CreateResource.h
  LoadResource.h
  Manager.h
  Metadata.h
  MetadataContainer.h
  MetadataObserver.h
  Observer.h
  Operation.h
  RegisterOperations.h
  RegisterResources.h
  ResourceManagerOperation.h
  SaveResource.h
  XMLOperation.h
)

smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/Operation.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/CreateResource.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/LoadResource.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/SaveResource.sbt" defOpXML)

if (SMTK_ENABLE_PYTHON_WRAPPING)

  list (APPEND operationSrcs ImportPythonOperation.cxx)
  list (APPEND operationHeaders ImportPythonOperation.h)
  list (APPEND operationSrcs RegisterPythonOperations.cxx)
  list (APPEND operationHeaders RegisterPythonOperations.h)

  add_subdirectory(pybind11)

endif()


#install the headers
smtk_public_headers(${operationHeaders})

add_subdirectory(environment)

if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_operation
    PUBLIC nlohmann_json)
  add_subdirectory(testing)
endif()

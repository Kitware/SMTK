set(operationSrcs
  CreateResource.cxx
  LoadResource.cxx
  Manager.cxx
  Metadata.cxx
  MetadataObserver.cxx
  Observer.cxx
  Operator.cxx
  NewOp.cxx
  RegisterOperations.cxx
  RegisterResources.cxx
  ResourceManagerOperator.cxx
  SaveResource.cxx
  SpecificationOps.cxx
  XMLOperator.cxx
)

set(operationHeaders
  CreateResource.h
  LoadResource.h
  Manager.h
  Metadata.h
  MetadataContainer.h
  MetadataObserver.h
  Observer.h
  Operator.h
  NewOp.h
  RegisterOperations.h
  RegisterResources.h
  ResourceManagerOperator.h
  SaveResource.h
  XMLOperator.h
)

smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/NewOp.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/CreateResource.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/LoadResource.sbt" defOpXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/SaveResource.sbt" defOpXML)

if (SMTK_ENABLE_PYTHON_WRAPPING)

  list (APPEND operationSrcs ImportPythonOperator.cxx)
  list (APPEND operationHeaders ImportPythonOperator.h)
  list (APPEND operationSrcs RegisterPythonOperators.cxx)
  list (APPEND operationHeaders RegisterPythonOperators.h)

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

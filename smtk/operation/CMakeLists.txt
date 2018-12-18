set(operationSrcs
  Launcher.cxx
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

  groups/CreatorGroup.cxx
  groups/ImporterGroup.cxx
  groups/ExporterGroup.cxx
  groups/ReaderGroup.cxx
  groups/ResourceIOGroup.cxx
  groups/WriterGroup.cxx

  operators/ImportResource.cxx
  operators/ReadResource.cxx
  operators/RemoveResource.cxx
  operators/WriteResource.cxx
)

set(operationHeaders
  Launcher.h
  Group.h
  Manager.h
  Metadata.h
  MetadataContainer.h
  MetadataObserver.h
  Observer.h
  Operation.h
  Registrar.h
  ResourceManagerOperation.h
  SpecificationOps.h
  XMLOperation.h

  groups/CreatorGroup.h
  groups/ImporterGroup.h
  groups/ExporterGroup.h
  groups/ReaderGroup.h
  groups/ResourceIOGroup.h
  groups/WriterGroup.h

  operators/ImportResource.h
  operators/ReadResource.h
  operators/RemoveResource.h
  operators/WriteResource.h
)

set(operationXML
  Operation.xml
  Result.xml
  )

smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/Operation.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ReadResource.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/RemoveResource.sbt" defOpXML)
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
smtk_public_headers(smtkCore ${operationHeaders})

#also install the xml files
install(
  FILES ${operationXML}
  DESTINATION include/smtk/${SMTK_VERSION}/smtk/operation
  )

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_smtk_plugin(
    smtkOperationPlugin "1.0"
    REGISTRAR smtk::operation::Registrar
    MANAGERS smtk::operation::Manager smtk::resource::Manager
    LIBRARIES smtkCore)
  smtk_install_library(smtkOperationPlugin)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

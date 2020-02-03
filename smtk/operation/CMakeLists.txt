set(operationSrcs
  Launcher.cxx
  MarkGeometry.cxx
  Group.cxx
  Manager.cxx
  Metadata.cxx
  Operation.cxx
  Registrar.cxx
  ResourceManagerOperation.cxx
  SpecificationOps.cxx
  XMLOperation.cxx

  groups/CreatorGroup.cxx
  groups/ImporterGroup.cxx
  groups/InternalGroup.cxx
  groups/ExporterGroup.cxx
  groups/ReaderGroup.cxx
  groups/ResourceIOGroup.cxx
  groups/WriterGroup.cxx

  operators/ImportResource.cxx
  operators/ReadResource.cxx
  operators/RemoveResource.cxx
  operators/SetProperty.cxx
  operators/WriteResource.cxx
)

set(operationHeaders
  Launcher.h
  MarkGeometry.h
  Group.h
  GroupObserver.h
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
  groups/InternalGroup.h
  groups/ExporterGroup.h
  groups/ReaderGroup.h
  groups/ResourceIOGroup.h
  groups/WriterGroup.h

  operators/ImportResource.h
  operators/ReadResource.h
  operators/RemoveResource.h
  operators/SetProperty.h
  operators/WriteResource.h
)

set(operationXML
  Operation.xml
  Result.xml
  )

smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/Operation.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/ReadResource.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/RemoveResource.sbt" defOpXML)
smtk_operation_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/SetProperty.sbt" defOpXML)
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
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/smtk/${SMTK_VERSION}/smtk/operation
  )
foreach (operation_xml IN LISTS operationXML)
  configure_file(
    "${operation_xml}"
    "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}/smtk/${SMTK_VERSION}/smtk/operation/${operation_xml}"
    COPYONLY)
endforeach ()

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

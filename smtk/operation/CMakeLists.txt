set(operationSrcs
  Group.cxx
  GroupOps.cxx
  Helper.cxx
  Launcher.cxx
  MarkGeometry.cxx
  Manager.cxx
  Metadata.cxx
  Operation.cxx
  Registrar.cxx
  ResultOps.cxx
  SpecificationOps.cxx
  XMLOperation.cxx

  groups/ArcCreator.cxx
  groups/ArcDeleter.cxx
  groups/CreatorGroup.cxx
  groups/DeleterGroup.cxx
  groups/GroupingGroup.cxx
  groups/ImporterGroup.cxx
  groups/InternalGroup.cxx
  groups/ExporterGroup.cxx
  groups/NamingGroup.cxx
  groups/ReaderGroup.cxx
  groups/ResourceIOGroup.cxx
  groups/UngroupingGroup.cxx
  groups/WriterGroup.cxx

  operators/ImportResource.cxx
)

set(operationHeaders
  Launcher.h
  MarkGeometry.h
  Group.h
  GroupObserver.h
  GroupOps.h
  Helper.h
  Hints.h
  Manager.h
  Metadata.h
  MetadataContainer.h
  MetadataObserver.h
  Observer.h
  Operation.h
  Registrar.h
  ResultOps.h
  SpecificationOps.h
  XMLOperation.h

  groups/ArcCreator.h
  groups/ArcDeleter.h
  groups/CreatorGroup.h
  groups/DeleterGroup.h
  groups/GroupingGroup.h
  groups/ImporterGroup.h
  groups/InternalGroup.h
  groups/ExporterGroup.h
  groups/NamingGroup.h
  groups/ReaderGroup.h
  groups/ResourceIOGroup.h
  groups/UngroupingGroup.h
  groups/WriterGroup.h

  operators/ImportResource.h

  queries/SynchronizedCache.h
)

set(operationXML
  Hints.xml
  Operation.xml
  Result.xml
)

set(operationOperators
  AssignColors
  CoordinateTransform
  CopyResources
  EditProperties
  MarkModified
  ReadResource
  RemoveResource
  SetProperty
  WriteResource
)

set(icons
  assign_colors
  import_python_operation
  edit_properties
)
foreach(icon ${icons})
  smtk_encode_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/icons/${icon}.svg" TYPE "_svg" TARGET_OUTPUT targetName)
  list(APPEND smtkOperationDependencies ${targetName})
endforeach()

smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/Operation.sbt"
  HEADER_OUTPUT headerName)
list(APPEND _operationDependencies ${headerName})
foreach (operator ${operationOperators})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
    HEADER_OUTPUT headerName)
  list(APPEND operationSrcs operators/${operator}.cxx)
  list(APPEND operationHeaders operators/${operator}.h)
  list(APPEND _operationDependencies ${headerName})
endforeach()
add_custom_target(operationGenHeaders
  DEPENDS
    ${_operationDependencies}
    ${smtkOperationDependencies}
)
# set(operationDependencies ${_operationDependencies} PARENT_SCOPE)

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

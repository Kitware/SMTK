set(operationClasses
  Group
  GroupOps
  GroupObserver
  Helper
  Hints
  Launcher
  MarkGeometry
  JobSpecs
  Manager
  Metadata
  MetadataContainer
  MetadataObserver
  Observer
  Operation
  Registrar
  ResultOps
  SpecificationOps
  XMLOperation

  groups/ArcCreator
  groups/ArcDeleter
  groups/CreatorGroup
  groups/DeleterGroup
  groups/ExporterGroup
  groups/GroupingGroup
  groups/ImporterGroup
  groups/InternalGroup
  groups/NamingGroup
  groups/ReaderGroup
  groups/ResourceIOGroup
  groups/UngroupingGroup
  groups/WriterGroup

  queries/SynchronizedCache
)

set(operationSrcs)
set(operationHeaders)

foreach(class ${operationClasses})
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${class}.h")
    list(APPEND operationHeaders "${class}.h")
  endif()
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${class}.cxx")
    list(APPEND operationSrcs "${class}.cxx")
  endif()
endforeach()

set(operationXML
  Hints.xml
  Operation.xml
  Result.xml
  JobSpecs.xml
)

set(operationOperators
  AssignColors
  CoordinateTransform
  CopyResources
  EditProperties
  ImportResource
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
  list(APPEND operationHeaders operators/${operator}.h)
  list(APPEND operationSrcs operators/${operator}.cxx)
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt")
    smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
      HEADER_OUTPUT headerName)
    list(APPEND _operationDependencies ${headerName})
  endif()
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
  COMPONENT Development
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

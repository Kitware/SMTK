set(projectSrcs
  Manager.cxx
  Operation.cxx
  OperationFactory.cxx
  Project.cxx
  Registrar.cxx
  ResourceContainer.cxx

  json/jsonOperationFactory.cxx
  json/jsonProject.cxx
  json/jsonResourceContainer.cxx

  view/IconConstructor.cxx
  view/PhraseContent.cxx
  view/PhraseModel.cxx
  view/SubphraseGenerator.cxx
  )

set(projectHeaders
  Container.h
  Manager.h
  Metadata.h
  MetadataContainer.h
  MetadataObserver.h
  Observer.h
  Operation.h
  OperationFactory.h
  Project.h
  Registrar.h
  ResourceContainer.h
  Tags.h

  json/jsonOperationFactory.h
  json/jsonProject.h
  json/jsonResourceContainer.h

  view/IconConstructor.h
  view/PhraseContent.h
  view/PhraseModel.h
  view/SubphraseGenerator.h
)

set(iconFiles
  view/icons/project.svg
)

foreach (iconFile ${iconFiles})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/${iconFile}"
    TYPE "_svg"
    HEADER_OUTPUT iconHeaderFile
  )
  list(APPEND projectHeaders ${iconHeaderFile})
endforeach ()

set(projectOperators
  Add
  Create
  Define
  Print
  Read
  Remove
  Write
)

foreach (operator ${projectOperators})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
    HEADER_OUTPUT headerName
    TARGET_OUTPUT headerTarget
  )
  list(APPEND projectSrcs operators/${operator}.cxx)
  list(APPEND projectHeaders operators/${operator}.h)
  list(APPEND projectHeaders ${headerName})
  list(APPEND projectDependencies ${headerTarget})
endforeach ()
add_custom_target(projectGenHeaders DEPENDS ${_projectDependencies} ${projectHeaders})
set(projectDependencies ${_projectDependencies} PARENT_SCOPE)

# Install the headers
smtk_public_headers(smtkCore ${projectHeaders})

if (SMTK_ENABLE_PYTHON_WRAPPING)
  list(APPEND projectSrcs
      RegisterPythonProject.cxx)
  list(APPEND projectHeaders
      RegisterPythonProject.h)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

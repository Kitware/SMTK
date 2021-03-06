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

include(EncodeStringFunctions)

foreach (iconFile ${iconFiles})
  get_filename_component(genFileBase "${iconFile}" NAME_WE)
  set(genFile "${CMAKE_CURRENT_BINARY_DIR}/view/icons/${genFileBase}_svg.h")
  configureFileAsCVariable("${CMAKE_CURRENT_SOURCE_DIR}/${iconFile}" "${genFile}" "${genFileBase}_svg" "")
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
  HEADER_OUTPUT headerName)
  list(APPEND projectSrcs operators/${operator}.cxx)
  list(APPEND projectHeaders operators/${operator}.h)
  list(APPEND _projectDependencies ${headerName})
endforeach ()
add_custom_target(projectGenHeaders DEPENDS ${_projectDependencies})
set(projectDependencies ${_projectDependencies} PARENT_SCOPE)

# Install the headers
smtk_public_headers(smtkCore ${projectHeaders})

cmake_dependent_option(
  SMTK_ENABLE_PROJECT_UI "Build plugin with smtk::project UI elements" ON SMTK_ENABLE_PARAVIEW_SUPPORT OFF)
mark_as_advanced(SMTK_ENABLE_PROJECT_UI)

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

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

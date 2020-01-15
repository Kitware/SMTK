# Set up sources to build
set(jsonHeaders
  old/json/jsonProjectDescriptor.h
  old/json/jsonResourceDescriptor.h
)

set(jsonSrcs
  old/json/jsonProjectDescriptor.cxx
  old/json/jsonResourceDescriptor.cxx
)

set(projectSrcs
  ${jsonSrcs}
  old/Manager.cxx
  old/Project.cxx

  Manager.cxx
  Operation.cxx
  OperationFactory.cxx
  PhraseContent.cxx
  PhraseModel.cxx
  Project.cxx
  Registrar.cxx
  ResourceContainer.cxx

  json/jsonOperationFactory.cxx
  json/jsonProject.cxx
  json/jsonResourceContainer.cxx
  )

set(projectHeaders
  ${jsonHeaders}
  old/Manager.h
  old/Project.h
  old/ProjectDescriptor.h
  old/ResourceDescriptor.h

  Manager.h
  Metadata.h
  MetadataContainer.h
  Operation.h
  OperationFactory.h
  PhraseContent.h
  PhraseModel.h
  Project.h
  Registrar.h
  ResourceContainer.h
  Tags.h

  json/jsonOperationFactory.h
  json/jsonProject.h
  json/jsonResourceContainer.h
)

set(iconFiles
  icons/project.svg
)

include(EncodeStringFunctions)

configureFileAsCVariable(
  "${CMAKE_CURRENT_SOURCE_DIR}/old/NewProject.sbt"
  "${CMAKE_CURRENT_BINARY_DIR}/old/NewProjectTemplate.h"
  "NewProjectTemplate"
  "")

foreach (iconFile ${iconFiles})
  get_filename_component(genFileBase "${iconFile}" NAME_WE)
  set(genFile "${CMAKE_CURRENT_BINARY_DIR}/icons/${genFileBase}_svg.h")
  configureFileAsCVariable("${CMAKE_CURRENT_SOURCE_DIR}/${iconFile}" "${genFile}" "${genFileBase}_svg" "")
endforeach()

set(projectOperators
  AddToProject
  CreateProject
  DefineProject
  )

foreach (operator ${projectOperators})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
  HEADER_OUTPUT headerName)
  list(APPEND projectSrcs operators/${operator}.cxx)
  list(APPEND projectHeaders operators/${operator}.h)
  list(APPEND _projectDependencies ${headerName})
endforeach()
add_custom_target(projectGenHeaders DEPENDS ${_projectDependencies})
set(projectDependencies ${_projectDependencies} PARENT_SCOPE)

# Install the headers
smtk_public_headers(smtkCore ${projectHeaders})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(old/pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(old/testing)
  add_subdirectory(testing)
endif()

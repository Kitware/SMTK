# Set up sources to build
set(jsonHeaders
  json/jsonProjectDescriptor.h
  json/jsonResourceDescriptor.h
)

set(jsonSrcs
  json/jsonProjectDescriptor.cxx
  json/jsonResourceDescriptor.cxx
)

set(projectSrcs
  ${jsonSrcs}
  Manager.cxx
  Project.cxx
)

set(projectHeaders
  ${jsonHeaders}
  Manager.h
  Project.h
  ProjectDescriptor.h
  ResourceDescriptor.h
)

configureFileAsCVariable(
  "${CMAKE_CURRENT_SOURCE_DIR}/NewProject.sbt"
  "${CMAKE_CURRENT_BINARY_DIR}/NewProjectTemplate.h"
  "NewProjectTemplate"
  "")

# Install the headers
smtk_public_headers(smtkCore ${projectHeaders})

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

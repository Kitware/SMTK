# Set up sources to build
set(jsonHeaders
  json/jsonProjectDescriptor.h
  json/jsonResourceDescriptor.h
)

set(projectSrcs
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

# Future:
# if (SMTK_ENABLE_PARAVIEW_SUPPORT)
#   add_smtk_plugin(
#     smtkProjectPlugin "1.0"
#     REGISTRAR smtk::project::Registrar
#     MANAGERS smtk::operation::Manager smtk::resource::Manager
#     LIBRARIES smtkCore)
#   smtk_install_library(smtkProjectPlugin)
# endif()

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

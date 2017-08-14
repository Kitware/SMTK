# set up sources to build
set(simulationSrcs
  ExportSpec.cxx
  UserData.cxx
)

set(simulationHeaders
  ExportSpec.h
  UserData.h
)

#install the headers
smtk_public_headers(${simulationHeaders})

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

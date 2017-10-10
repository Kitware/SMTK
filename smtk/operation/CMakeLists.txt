set(operationSrcs
  Manager.cxx
  Operator.cxx
)

set(operationHeaders
  Manager.h
  Operator.h
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

#install the headers
smtk_public_headers(${operationHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

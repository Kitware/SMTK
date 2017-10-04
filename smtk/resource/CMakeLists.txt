# set up sources to build
set(resourceSrcs
  Resource.cxx
  Component.cxx
  Set.cxx
)

set(resourceHeaders
  Resource.h
  Component.h
  Set.h
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

#install the headers
smtk_public_headers(${resourceHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

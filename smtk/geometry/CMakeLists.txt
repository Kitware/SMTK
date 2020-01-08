set(geometrySrcs
  Resource.cxx
  Manager.cxx
)

set(geometryHeaders
  Backend.h
  Cache.h
  Generator.h
  Geometry.h
  GeometryForBackend.h
  Manager.h
  Resource.h
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

#install the headers
smtk_public_headers(smtkCore ${geometryHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

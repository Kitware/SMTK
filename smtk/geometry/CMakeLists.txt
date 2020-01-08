set(geometrySrcs
  Resource.cxx
  Manager.cxx
)

set(geometryHeaders
  CachingGeometryProvider.h
  Backend.h
  Geometry.h
  GeometrForFormat.h
  Manager.h
  Resource.h
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

#install the headers
smtk_public_headers(smtkCore ${resourceHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

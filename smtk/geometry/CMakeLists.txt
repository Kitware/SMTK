set(geometrySrcs
  Geometry.cxx
  Registrar.cxx
  Resource.cxx
  Manager.cxx
  queries/BoundingBox.cxx
)

set(geometryHeaders
  Backend.h
  Cache.h
  Generator.h
  Geometry.h
  GeometryForBackend.h
  Manager.h
  Registrar.h
  Resource.h
  queries/BoundingBox.h
  queries/ClosestPoint.h
  queries/DistanceTo.h
  queries/RandomPoint.h
  queries/SelectionFootprint.h
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

#install the headers
smtk_public_headers(smtkCore ${geometryHeaders})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

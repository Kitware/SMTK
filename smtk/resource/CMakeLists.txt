# set up sources to build
set(resourceSrcs
  Component.cxx
  Lock.cxx
  Manager.cxx
  PersistentObject.cxx
  Resource.cxx
  Set.cxx
  json/jsonResource.cxx
)

set(resourceHeaders
  Component.h
  DerivedFrom.h
  Event.h
  Lock.h
  Manager.h
  Metadata.h
  PersistentObject.h
  PropertyType.h
  Registrar.h
  Resource.h
  Resource.h
  Container.h
  Set.h
  json/jsonResource.h
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

#install the headers
smtk_public_headers(${resourceHeaders})

if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_resource
    PUBLIC
    nlohmann_json)
  add_subdirectory(testing)
endif()

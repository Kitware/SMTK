# set up sources to build
set(resourceSrcs
  Component.cxx
  ComponentLinks.cxx
  Lock.cxx
  Manager.cxx
  Observer.cxx
  PersistentObject.cxx
  Resource.cxx
  ResourceLinks.cxx
  Set.cxx
  Surrogate.cxx
  json/jsonLinkBase.cxx
  json/jsonResource.cxx
  json/jsonSurrogate.cxx
)

set(resourceHeaders
  Component.h
  ComponentLinks.h
  DerivedFrom.h
  Lock.h
  Manager.h
  Metadata.h
  MetadataContainer.h
  Observer.h
  PersistentObject.h
  PropertyType.h
  Registrar.h
  Resource.h
  ResourceLinks.h
  Container.h
  Set.h
  Surrogate.h
  json/jsonLinkBase.h
  json/jsonResource.h
  json/jsonSurrogate.h
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

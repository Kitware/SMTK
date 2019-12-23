# set up sources to build
set(resourceSrcs
  Component.cxx
  ComponentLinks.cxx
  GarbageCollector.cxx
  Links.cxx
  Lock.cxx
  Manager.cxx
  PersistentObject.cxx
  Properties.cxx
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
  Container.h
  DerivedFrom.h
  GarbageCollector.h
  Links.h
  Lock.h
  Manager.h
  Metadata.h
  MetadataContainer.h
  MetadataObserver.h
  Observer.h
  PersistentObject.h
  Properties.h
  PropertyType.h
  Registrar.h
  Resource.h
  ResourceLinks.h
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
smtk_public_headers(smtkCore ${resourceHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

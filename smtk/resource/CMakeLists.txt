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
  Registrar.cxx
  Resource.cxx
  ResourceLinks.cxx
  Surrogate.cxx
  json/Helper.cxx
  json/jsonComponentLinkBase.cxx
  json/jsonPropertyCoordinateFrame.cxx
  json/jsonResource.cxx
  json/jsonResourceLinkBase.cxx
  json/jsonSurrogate.cxx
  properties/CoordinateFrame.cxx
  query/Factory.cxx
  query/Manager.cxx
  query/Query.cxx
)

set(resourceHeaders
  Component.h
  ComponentLinks.h
  Container.h
  DerivedFrom.h
  GarbageCollector.h
  LinkInformation.h
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
  Surrogate.h
  filter/Action.h
  filter/Enclosed.h
  filter/Filter.h
  filter/FloatingPoint.h
  filter/Grammar.h
  filter/Integer.h
  filter/Name.h
  filter/Property.h
  filter/Rule.h
  filter/Rules.h
  filter/String.h
  filter/Vector.h
  json/Helper.h
  json/jsonComponentLinkBase.h
  json/jsonPropertyCoordinateFrame.h
  json/jsonResource.h
  json/jsonResourceLinkBase.h
  json/jsonSurrogate.h
  properties/CoordinateFrame.h
  query/BadTypeError.h
  query/Cache.h
  query/Container.h
  query/Factory.h
  query/DerivedFrom.h
  query/Manager.h
  query/Metadata.h
  query/Queries.h
  query/Query.h
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  list (APPEND resourceSrcs
    RegisterPythonResource.cxx)
  list (APPEND resourceHeaders
    RegisterPythonResource.h)

  add_subdirectory(pybind11)
endif()

#install the headers
smtk_public_headers(smtkCore ${resourceHeaders})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

# set up sources to build
set(resourceSrcs
  Component.cxx
  Lock.cxx
  Manager.cxx
  RegisterOperations.cxx
  RegisterResources.cxx
  Resource.cxx
  Set.cxx
  json/jsonResource.cxx
)

set(resourceHeaders
  Component.h
  Event.h
  Lock.h
  Manager.h
  Metadata.h
  Name.h
  PropertyType.h
  RegisterOperations.h
  RegisterResources.h
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

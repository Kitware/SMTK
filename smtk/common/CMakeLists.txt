# set up sources to build
set(commonSrcs
  Resource.cxx
  ResourceSet.cxx
  UUID.cxx
  UUIDGenerator.cxx
)

set(commonHeaders
  Resource.h
  ResourceSet.h
  UUID.h
  UUIDGenerator.h
)

#install the headers
smtk_public_headers(${commonHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

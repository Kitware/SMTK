# set up sources to build
set(commonSrcs
  Environment.cxx
  Resource.cxx
  ResourceSet.cxx
  UUID.cxx
  UUIDGenerator.cxx
)

set(commonHeaders
  Environment.h
  Resource.h
  ResourceSet.h
  UUID.h
  UUIDGenerator.h
)

# Generate the version header using CMake variables.
configure_file(
  ${SMTK_SOURCE_DIR}/CMake/Version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/Version.h
  @ONLY
)

#install the headers
smtk_public_headers(${commonHeaders})

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/Version.h
  DESTINATION include/smtk/common
  COMPONENT Development
)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

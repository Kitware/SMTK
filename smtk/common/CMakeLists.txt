# set up sources to build
set(commonSrcs
  Environment.cxx
  FileLocation.cxx
  Paths.cxx
  Resource.cxx
  ResourceSet.cxx
  StringUtil.cxx
  UUID.cxx
  UUIDGenerator.cxx
  View.cxx
)

set(commonHeaders
  Environment.h
  FileLocation.h
  GeometryUtilities.h
  Paths.h
  RangeDetector.h
  Resource.h
  ResourceSet.h
  StringUtil.h
  UUID.h
  UUIDGenerator.h
  View.h
  ${CMAKE_CURRENT_BINARY_DIR}/Version.h
)

if (APPLE)
  set(commonSrcs ${commonSrcs} PathsHelperMacOSX.mm)
  set(commonHeaders ${commonHeaders} PathsHelperMacOSX.h)
endif()
if (UNIX)
  set(commonSrcs ${commonSrcs} PathsHelperUnix.cxx)
  set(commonHeaders ${commonHeaders} PathsHelperUnix.h)
endif()
if (WIN32)
  set(commonSrcs ${commonSrcs} PathsHelperWindows.cxx)
  set(commonHeaders ${commonHeaders} PathsHelperWindows.h)
endif()

# Generate the version header using CMake variables.
configure_file(
  ${SMTK_SOURCE_DIR}/CMake/Version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/Version.h
  @ONLY
)

#install the headers
smtk_public_headers(${commonHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

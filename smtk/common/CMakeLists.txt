# set up sources to build
set(commonSrcs
  Archive.cxx
  Color.cxx
  DateTime.cxx
  DateTimeZonePair.cxx
  Environment.cxx
  Extension.cxx
  FileLocation.cxx
  json/jsonLinks.cxx
  json/jsonProperties.cxx
  json/jsonUUID.cxx
  Paths.cxx
  Registry.cxx
  StringUtil.cxx
  TimeZone.cxx
  UUID.cxx
  UUIDGenerator.cxx
)

set(commonHeaders
  Archive.h
  Color.h
  CompilerInformation.h
  DateTime.h
  DateTimeZonePair.h
  Environment.h
  Extension.h
  FileLocation.h
  Generator.h
  GeometryUtilities.h
  json/jsonLinks.h
  json/jsonProperties.h
  json/jsonUUID.h
  Links.h
  Observers.h
  Paths.h
  Properties.h
  RangeDetector.h
  Registry.h
  Singleton.h
  StringUtil.h
  ThreadPool.h
  TimeZone.h
  TypeName.h
  UUID.h
  UUIDGenerator.h

  json/jsonUUID.h

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
  ${smtk_SOURCE_DIR}/CMake/Version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/Version.h
  @ONLY
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  list(APPEND commonSrcs
    PythonInterpreter.cxx
    )
  list(APPEND commonHeaders
    PythonInterpreter.h
    )

  add_subdirectory(pybind11)
endif()

#install the headers
smtk_public_headers(smtkCore ${commonHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

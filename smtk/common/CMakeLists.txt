# set up sources to build
set(commonSrcs
  Archive.cxx
  Categories.cxx
  Color.cxx
  DateTime.cxx
  DateTimeZonePair.cxx
  Environment.cxx
  Extension.cxx
  FileLocation.cxx
  InfixExpressionGrammar.cxx
  Managers.cxx
  Paths.cxx
  RuntimeTypeContainer.cxx
  Status.cxx
  StringUtil.cxx
  TimeZone.cxx
  timezonespec.cxx
  TypeContainer.cxx
  URL.cxx
  UUID.cxx
  UUIDGenerator.cxx
  VersionNumber.cxx
  json/Helper.cxx
  json/jsonLinks.cxx
  json/jsonUUID.cxx
  json/jsonVersionNumber.cxx
)

set(commonHeaders
  Archive.h
  Categories.h
  Color.h
  CompilerInformation.h
  DateTime.h
  DateTimeZonePair.h
  Deprecation.h
  Environment.h
  Extension.h
  Factory.h
  FileLocation.h
  Generator.h
  GeometryUtilities.h
  InfixExpressionError.h
  InfixExpressionEvaluation.h
  InfixExpressionGrammar.h
  InfixExpressionGrammarImpl.h
  Instances.h
  Links.h
  Managers.h
  Observers.h
  Paths.h
  Processing.h
  RangeDetector.h
  RuntimeTypeContainer.h
  Singleton.h
  Status.h
  StringUtil.h
  ThreadPool.h
  TimeZone.h
  timezonespec.h
  TypeHierarchy.h
  TypeMap.h
  TypeName.h
  TypeContainer.h
  TypeTraits.h
  URL.h
  UUID.h
  UUIDGenerator.h
  VersionNumber.h
  VersionMacros.h
  Visit.h
  WeakReferenceWrapper.h
  categories/Actions.h
  categories/Evaluators.h
  categories/Grammar.h
  json/Helper.h
  json/jsonLinks.h
  json/jsonTypeMap.h
  json/jsonUUID.h
  json/jsonVersionNumber.h
  testing/cxx/helpers.h

  update/Factory.h

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
  "${CMAKE_CURRENT_SOURCE_DIR}/Version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Version.h"
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

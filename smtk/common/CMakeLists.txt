# set up sources to build
set(commonClasses
  Archive
  Categories
  Color
  CompilerInformation
  DateTime
  DateTimeZonePair
  Environment
  Extension
  Factory
  FileLocation
  Generator
  GeometryUtilities
  InfixExpressionError
  InfixExpressionEvaluation
  InfixExpressionGrammar
  InfixExpressionGrammarImpl
  Instances
  Links
  Managers
  Observers
  Paths
  RangeDetector
  RuntimeTypeContainer
  Singleton
  Singletons
  Status
  StringUtil
  ThreadPool
  TimeZone
  timezonespec
  TypeHierarchy
  TypeMap
  TypeName
  TypeContainer
  TypeTraits
  URL
  UUID
  UUIDGenerator
  VersionNumber
  WeakReferenceWrapper

  categories/Actions
  categories/Evaluators
  categories/Grammar

  json/Helper
  json/jsonLinks
  json/jsonTypeMap
  json/jsonUUID
  json/jsonVersionNumber

  testing/cxx/helpers

  update/Factory
)

set(commonSrcs
)

set(commonHeaders
  Deprecation.h
  Processing.h
  VersionMacros.h
  Visit.h

  ${CMAKE_CURRENT_BINARY_DIR}/Version.h
)

foreach(class ${commonClasses})
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${class}.h")
    list(APPEND commonHeaders "${class}.h")
  endif()
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${class}.cxx")
    list(APPEND commonSrcs "${class}.cxx")
  endif()
endforeach()

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

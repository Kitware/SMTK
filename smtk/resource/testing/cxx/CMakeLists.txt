################################################################################
# Make a library of some utility functions for tests to use:
################################################################################
add_library(smtkCoreResourceTesting helpers.cxx)
smtk_export_header(smtkCoreResourceTesting CoreResourceTestingExports.h)
smtk_public_headers(smtkCore helpers.h ${CMAKE_CURRENT_BINARY_DIR}/CoreResourceTestingExports.h)
target_link_libraries(smtkCoreResourceTesting LINK_PRIVATE smtkCore)
smtk_install_library(smtkCoreResourceTesting)

################################################################################
# Tests
################################################################################
set(unit_tests
  TestGarbageCollector.cxx
  TestQuery.cxx
  TestLock.cxx
  TestResourceFilter.cxx
  TestResourceLinks.cxx
  TestResourceManager.cxx
  TestResourceProperties.cxx
  TestResourceQueries.cxx
)

smtk_unit_tests(
  LABEL "Resource"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)

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
  TestResourceManager.cxx
)

smtk_unit_tests(
  Label "Resource"
  SOURCES ${unit_tests}
  LIBRARIES smtkCore
)

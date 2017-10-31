################################################################################
# Make a library of some utility functions for tests to use:
################################################################################
add_library(smtkCoreResourceTesting STATIC helpers.cxx)
smtk_public_headers(helpers.h)
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

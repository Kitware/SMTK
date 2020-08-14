set(CTEST_USE_LAUNCHERS "ON" CACHE STRING "")

set(SMTK_PUBLIC_DROP_SITE "ON" CACHE BOOL "")
set(SMTK_ENABLE_TESTING "ON" CACHE BOOL "")
set(SMTK_ENABLE_EXAMPLES "ON" CACHE BOOL "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_sccache.cmake")

# Include the superbuild settings.
include("$ENV{SUPERBUILD_PREFIX}/smtk-developer-config.cmake")

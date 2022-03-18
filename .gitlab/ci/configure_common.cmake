set(CTEST_USE_LAUNCHERS "ON" CACHE STRING "")

set(SMTK_PUBLIC_DROP_SITE "ON" CACHE BOOL "")
set(SMTK_ENABLE_TESTING "ON" CACHE BOOL "")
set(SMTK_ENABLE_EXAMPLES "ON" CACHE BOOL "")

# Build binaries that will run on older architectures
if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "fedora")
  set(CMAKE_C_FLAGS "-march=core2 -mno-avx512f -Wall -Wextra" CACHE STRING "")
  set(CMAKE_CXX_FLAGS "-march=core2 -mno-avx512f -Wall -Wextra" CACHE STRING "")
endif ()

include("${CMAKE_CURRENT_LIST_DIR}/configure_sccache.cmake")

# Include the superbuild settings.
include("$ENV{SUPERBUILD_PREFIX}/smtk-developer-config.cmake")

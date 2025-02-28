set(CMAKE_C_CLANG_TIDY "clang-tidy-cache" "--header-filter=$ENV{CI_PROJECT_DIR}" CACHE STRING "")
set(CMAKE_CXX_CLANG_TIDY "clang-tidy-cache" "--header-filter=$ENV{CI_PROJECT_DIR}" CACHE STRING "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora41.cmake")

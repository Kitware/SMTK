set(CMAKE_C_CLANG_TIDY "/usr/bin/clang-tidy" "--header-filter=$ENV{CI_PROJECT_DIR}" CACHE STRING "")
set(CMAKE_CXX_CLANG_TIDY "/usr/bin/clang-tidy" "--header-filter=$ENV{CI_PROJECT_DIR}" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora41.cmake")

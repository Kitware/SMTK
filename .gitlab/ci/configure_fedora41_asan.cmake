set(SMTK_ENABLE_SANITIZER ON CACHE BOOL "")
set(SMTK_SANITIZER "address" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora41_paraview.cmake")

set(smtk_conda_build TRUE)

set(conda_prefix "$ENV{CONDA_PREFIX}")

set(Boost_NO_SYSTEM_PATHS ON CACHE BOOL "Initial cache")
set(Boost_NO_BOOST_CMAKE ON CACHE BOOL "Initial cache")
if ($ENV{CONDA_BUILD})
  # For packaging by conda-build
  set(BOOST_INCLUDEDIR "$ENV{PREFIX}/include" CACHE PATH "Initial cache")
  set(BOOST_LIBRARYDIR "$$ENV{PREFIX}/lib" CACHE PATH "Initial cache")
else()
  # For developer builds
  set(BOOST_INCLUDEDIR "${conda_prefix}/include" CACHE PATH "Initial cache")
  set(BOOST_LIBRARYDIR "${conda_prefix}/lib" CACHE PATH "Initial cache")
endif()

set(MOAB_ROOT_DIR "${conda_prefix}/lib/cmake/MOAB")

set(SMTK_ENABLE_EXODUS_SESSION OFF CACHE BOOL "Initial cache")
set(SMTK_ENABLE_MESH_SESSION ON CACHE BOOL "Initial cache")
set(SMTK_ENABLE_POLYGON_SESSION ON CACHE BOOL "Initial cache")
set(SMTK_ENABLE_VTK_SUPPORT OFF CACHE BOOL "Initial cache")

# Ignore problems linking executable tests
set(SMTK_ENABLE_TESTING OFF CACHE BOOL "Initial cache")

set(SMTK_ENABLE_PYTHON_WRAPPING ON CACHE BOOL "Initial cache")

set(conda_root "$ENV{CONDA_PREFIX}")
message("conda_root ${conda_root}")
if (conda_root)
  message("Using conda environment ${conda_root}")
else()
  message(FATAL_ERROR "CONDA_PREFIX not found. The conda environment must be set.")
endif()

set(Boost_NO_SYSTEM_PATHS ON CACHE BOOL "Initial cache")
set(Boost_NO_BOOST_CMAKE ON CACHE BOOL "Initial cache")
set(BOOST_INCLUDEDIR "${conda_root}/include" CACHE PATH "Initial cache")
set(BOOST_LIBRARYDIR "${conda_root}/lib" CACHE PATH "Initial cache")

set(SMTK_ENABLE_EXODUS_SESSION OFF CACHE BOOL "Initial cache")
set(SMTK_ENABLE_MESH_SESSION ON CACHE BOOL "Initial cache")
set(SMTK_ENABLE_POLYGON_SESSION ON CACHE BOOL "Initial cache")
#set(SMTK_ENABLE_TESTING OFF CACHE BOOL "Initial cache")
set(SMTK_ENABLE_VTK_SUPPORT OFF CACHE BOOL "Initial cache")

set(SMTK_ENABLE_PYTHON_WRAPPING ON CACHE BOOL "Initial cache")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/CondaToolchain.cmake" CACHE PATH "Initial cache")

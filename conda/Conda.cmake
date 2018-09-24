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
set(SMTK_ENABLE_VTK_SUPPORT OFF CACHE BOOL "Initial cache")

# Ignore problems linking executable tests
set(SMTK_ENABLE_TESTING OFF CACHE BOOL "Initial cache")

set(SMTK_ENABLE_PYTHON_WRAPPING ON CACHE BOOL "Initial cache")

# Override default CMAKE_INSTALL_PREFIX
if (IS_ABSOLUTE ${CMAKE_INSTALL_PREFIX})
  #message("IS_ABSOLUTE")
  if (conda_root)
    # Developer build
    set(CMAKE_INSTALL_PREFIX ${conda_root} CACHE PATH "Reset cache" FORCE)
  elseif (NOT ${CMAKE_INSTALL_PREFIX})
    # Package build
    set(CMAKE_INSTALL_PREFIX "install" CACHE PATH "Reset cache" FORCE)
  endif()
  message("Set CMAKE_INSTALL_PREFIX to ${CMAKE_INSTALL_PREFIX}")
endif()

# set(isabs IS_ABSOLUTE ${CMAKE_INSTALL_PREFIX})
# message("Before CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}, ISABS: ${isabs}")
# if (NOT CMAKE_INSTALL_PREFIX OR IS_ABSOLUTE ${CMAKE_INSTALL_PREFIX})
# endif()
message("After CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/CondaToolchain.cmake" CACHE PATH "Initial cache")

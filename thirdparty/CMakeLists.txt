add_subdirectory(cJSON)
if (NOT SMTK_USE_SYSTEM_MOAB)
  #guard against MOAB polluting the cmake C/CXX/Fortran flags
  set(save_CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
  set(save_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(save_CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS}")
  add_subdirectory(moab)

  set(CMAKE_C_FLAGS "${save_CMAKE_C_FLAGS}" CACHE STRING "" FORCE)
  set(CMAKE_CXX_FLAGS "${save_CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
  set(CMAKE_Fortran_FLAGS "${save_CMAKE_Fortran_FLAGS}" CACHE STRING "" FORCE)

  #hide all of the CMake options that moab has. This is done to keep
  #the smtk ccmake/cmake-gui interface simple.
  mark_as_advanced(
  ENABLE_CGM
  ENABLE_CGNS
  ENABLE_HDF5
  ENABLE_IGEOM
  ENABLE_IMESH
  ENABLE_METIS
  ENABLE_MPI
  ENABLE_NETCDF
  ENABLE_PARMETIS
  ENABLE_PNETCDF
  ENABLE_SZIP
  ENABLE_TESTING
  ENABLE_VTK
  ENABLE_ZOLTAN
  GZIP
  MOAB_BUILD_DAGMC
  MOAB_BUILD_HEXMODOPS
  MOAB_BUILD_MBCHACO
  MOAB_BUILD_MBCONVERT
  MOAB_BUILD_MBCOUPLER
  MOAB_BUILD_MBDEPTH
  MOAB_BUILD_MBGSETS
  MOAB_BUILD_MBMEM
  MOAB_BUILD_MBPART
  MOAB_BUILD_MBSIZE
  MOAB_BUILD_MBSKIN
  MOAB_BUILD_MBSURFPLOT
  MOAB_BUILD_MBTAGPROP
  MOAB_BUILD_QVDUAL
  MOAB_BUILD_SPHEREDECOMP
  MOAB_FORCE_32_BIT_HANDLES
  MOAB_FORCE_64_BIT_HANDLES
  SED
  TAR
  verdict_DIR
  )
endif()
# Only use the local sparsehash when the system version is not requested.
if (NOT SMTK_USE_SYSTEM_SPARSEHASH)
  add_subdirectory(sparsehash)
endif()

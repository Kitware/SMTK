
# Include ability to add flags unless already present
include (config/ForceAddFlags.cmake)

SET(MOAB_CXX_FLAGS "")

#
# Set -pedantic if the compiler supports it.
#
#IF(NOT (CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND
#        CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.4"))
#  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-pedantic")
#ENDIF()

# Check for compiler types and add flags accordingly
if (CMAKE_COMPILER_IS_GNUCXX)

  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-fpic")
  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-Wall")
  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-pipe")
  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-Wno-long-long")
  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-Wextra")
  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-Wcast-align")
  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-Wsign-compare")
  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-Wpointer-arith")
  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-Wformat")
  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-Wformat-security")
  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-Wunused-parameter")
  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-fstack-protector-all")
  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-mtune=native")
  FORCE_ADD_FLAGS(CMAKE_C_FLAGS "${MOAB_CXX_FLAGS}")
  FORCE_ADD_FLAGS(CMAKE_Fortran_FLAGS "${MOAB_CXX_FLAGS}")
  ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-fpermissive")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8)
    ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-Wno-unused-local-typedefs")
  endif()
  if(APPLE) # Clang / Mac OS only
    # Required on OSX to compile c++11
    ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-stdlib=libc++")
    ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-mmacosx-version-min=10.7")
  endif()
  # gfortran
  set (CMAKE_Fortran_FLAGS_RELEASE "-funroll-all-loops -fno-f2c -O3")
  set (CMAKE_Fortran_FLAGS_DEBUG   "-fno-f2c -O0 -g")
else (CMAKE_COMPILER_IS_GNUCXX)
  IF(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-ansi")  # standard compliant
    ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-w2")    # verbose warnings
    ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-Wall")
    # disable some warnings -- consistent with autoconf
    # -wd981 -wd279 -wd1418 -wd383 -wd1572 -wd2259
    ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-wd981")
    ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-wd279")
    ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-wd1418")
    ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-wd383")
    ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-wd1572")
    ENABLE_IF_SUPPORTED(MOAB_CXX_FLAGS "-wd2259")
    FORCE_ADD_FLAGS(CMAKE_C_FLAGS "${MOAB_CXX_FLAGS}")
    FORCE_ADD_FLAGS(CMAKE_Fortran_FLAGS "${MOAB_CXX_FLAGS}")
    # ifort (untested)
    set (CMAKE_Fortran_FLAGS_RELEASE "-f77rtl -O3")
    set (CMAKE_Fortran_FLAGS_DEBUG   "-f77rtl -O0 -g")
  else ()
    set (CMAKE_Fortran_FLAGS_RELEASE "-O2")
    set (CMAKE_Fortran_FLAGS_DEBUG   "-O0 -g")
  endif()
endif (CMAKE_COMPILER_IS_GNUCXX)

FORCE_ADD_FLAGS(CMAKE_CXX_FLAGS "${MOAB_CXX_FLAGS}")

# Debug targets
IF (CMAKE_BUILD_TYPE MATCHES "Debug")

  ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-Og")
  #
  # If -Og is not available, fall back to -O0:
  #
  IF(NOT CMAKE_HAVE_FLAG_Og)
    FORCE_ADD_FLAGS(CMAKE_CXX_FLAGS "-O0")
    FORCE_ADD_FLAGS(CMAKE_C_FLAGS "-O0")
  ENDIF()

  ENABLE_IF_SUPPORTED(CMAKE_C_FLAGS "-ggdb")
  ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-ggdb")
  ENABLE_IF_SUPPORTED(CMAKE_LINKER_FLAGS "-ggdb")
  #
  # If -ggdb is not available, fall back to -g:
  #
  IF(NOT CMAKE_HAVE_FLAG_ggdb)
    ENABLE_IF_SUPPORTED(CMAKE_C_FLAGS "-g")
    ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-g")
    ENABLE_IF_SUPPORTED(CMAKE_LINKER_FLAGS "-g")
  ENDIF()

  IF(CMAKE_SETUP_COVERAGE)
    #
    # Enable test coverage
    #
    ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-fno-elide-constructors")
    ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-ftest-coverage -fprofile-arcs")
    ENABLE_IF_SUPPORTED(CMAKE_LINKER_FLAGS "-ftest-coverage -fprofile-arcs")
  ENDIF()

ENDIF()

# Release targets
IF (CMAKE_BUILD_TYPE MATCHES "Release")
  #
  # General optimization flags:
  #
  FORCE_ADD_FLAGS(CMAKE_C_FLAGS "-O2")
  FORCE_ADD_FLAGS(CMAKE_CXX_FLAGS "-O2")

  ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-ip")
  ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-funroll-loops")
  ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-funroll-all-loops")
  ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-fstrict-aliasing")

  ENABLE_IF_SUPPORTED(CMAKE_CXX_FLAGS "-Wno-unused")
ENDIF()

mark_as_advanced(CMAKE_Fortran_FLAGS MOAB_CXX_FLAGS)


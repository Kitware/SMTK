#
# Find the native HDF5 includes and library
#
# HDF5_INCLUDES    - where to find hdf5.h, H5public.h, etc.
# HDF5_LIBRARIES   - List of fully qualified libraries to link against when using hdf5.
# HDF5_FOUND       - Do not attempt to use hdf5 if "no" or undefined.

set( HDF5_ROOT "" CACHE PATH "Path to search for HDF5 header and library files" )
set (HDF5_FOUND NO CACHE INTERNAL "Found HDF5 components successfully." )
set( SZIP_ROOT "" CACHE PATH "Path to search for SZIP header and library files" )

# Try to find HDF5 with the CMake finder
set(ENV{HDF5_ROOT} ${HDF5_ROOT})
find_package(HDF5 COMPONENTS C HL)

if (HDF5_FOUND)
  # Translate to MOAB variables
  SET(HDF5_INCLUDES ${HDF5_INCLUDE_DIRS})
  SET(HDF5_LIBRARIES ${HDF5_HL_LIBRARIES} ${HDF5_C_LIBRARIES})
else (HDF5_FOUND)
  # Try to find HDF5 ourselves
  if(EXISTS "${HDF5_ROOT}/share/cmake/hdf5/hdf5-config.cmake")
    include(${HDF5_ROOT}/share/cmake/hdf5/hdf5-config.cmake)
  else()

    FIND_PATH(HDF5_INCLUDE_DIR
      NAMES hdf5.h H5public.h
      HINTS ${HDF5_ROOT}/include
      HINTS ${HDF5_ROOT}
      ENV CPLUS_INCLUDE_PATH
      NO_DEFAULT_PATH
      )

    foreach (VARIANT dl sz z m )
      set (hdf5_deplibs_${VARIANT} "hdf5_deplibs_${VARIANT}-NOTFOUND" CACHE INTERNAL "HDF5 external library component ${VARIANT}." )
      FIND_LIBRARY(hdf5_deplibs_${VARIANT} ${VARIANT}
        HINTS ${HDF5_ROOT}/lib ${SZIP_ROOT}/lib /lib /lib64 /usr/local/lib /usr/lib /opt/local/lib
        )
      if (NOT ${hdf5_deplibs_${VARIANT}} MATCHES "(.*)NOTFOUND")
        list(APPEND HDF5_DEP_LIBRARIES ${hdf5_deplibs_${VARIANT}})
      endif (NOT ${hdf5_deplibs_${VARIANT}} MATCHES "(.*)NOTFOUND")
    endforeach()

    FIND_LIBRARY(HDF5_BASE_LIBRARY NAMES libhdf5.a libhdf5d.a hdf5 hdf5d
      HINTS ${HDF5_ROOT} ${HDF5_ROOT}/lib 
      )
    FIND_LIBRARY(HDF5_HLBASE_LIBRARY libhdf5_hl.a libhdf5_hld.a hdf5_hl hdf5_hld
      HINTS ${HDF5_ROOT} ${HDF5_ROOT}/lib
      )

    IF (NOT HDF5_FOUND)
      IF (HDF5_INCLUDE_DIR AND HDF5_BASE_LIBRARY)
        FIND_LIBRARY(HDF5_CXX_LIBRARY libhdf5_cxx.a hdf5_cxx
          HINTS ${HDF5_ROOT} ${HDF5_ROOT}/lib NO_DEFAULT_PATH
          )
        FIND_LIBRARY(HDF5_HLCXX_LIBRARY libhdf5_hl_cxx.a hdf5_hl_cxx
          HINTS ${HDF5_ROOT} ${HDF5_ROOT}/lib NO_DEFAULT_PATH
          )
        FIND_LIBRARY(HDF5_FORT_LIBRARY libhdf5_fortran.a hdf5_fortran
          HINTS ${HDF5_ROOT} ${HDF5_ROOT}/lib NO_DEFAULT_PATH
          )
        FIND_LIBRARY(HDF5_HLFORT_LIBRARY
          NAMES libhdf5hl_fortran.a libhdf5_hl_fortran.a hdf5hl_fortran hdf5_hl_fortran
          HINTS ${HDF5_ROOT} ${HDF5_ROOT}/lib NO_DEFAULT_PATH
          )
        SET( HDF5_INCLUDES "${HDF5_INCLUDE_DIR}" )
        if (HDF5_FORT_LIBRARY)
          FIND_PATH(HDF5_FORT_INCLUDE_DIR
            NAMES hdf5.mod
            HINTS ${HDF5_ROOT}/include
            ${HDF5_ROOT}/include/fortran
            )
          if (HDF5_FORT_INCLUDE_DIR AND NOT ${HDF5_FORT_INCLUDE_DIR} STREQUAL ${HDF5_INCLUDE_DIR})
            SET( HDF5_INCLUDES "${HDF5_INCLUDES} ${HDF5_FORT_INCLUDE_DIR}" )
          endif (HDF5_FORT_INCLUDE_DIR AND NOT ${HDF5_FORT_INCLUDE_DIR} STREQUAL ${HDF5_INCLUDE_DIR})
          unset(HDF5_FORT_INCLUDE_DIR CACHE)
        endif (HDF5_FORT_LIBRARY)
        # Add the libraries based on availability
        foreach (VARIANT CXX FORT BASE )
          if (HDF5_HL${VARIANT}_LIBRARY)
            list(APPEND HDF5_LIBRARIES ${HDF5_HL${VARIANT}_LIBRARY})
          endif (HDF5_HL${VARIANT}_LIBRARY)
          if (HDF5_${VARIANT}_LIBRARY)
            list(APPEND HDF5_LIBRARIES ${HDF5_${VARIANT}_LIBRARY})
          endif (HDF5_${VARIANT}_LIBRARY)
          unset(HDF5_HL${VARIANT}_LIBRARY CACHE)
          unset(HDF5_${VARIANT}_LIBRARY CACHE)
        endforeach()
        list(APPEND HDF5_LIBRARIES ${HDF5_DEP_LIBRARIES})
        # If the HDF5 include directory was found, open H5pubconf.h to determine if
        # HDF5 was compiled with parallel IO support 
        set( HDF5_IS_PARALLEL FALSE )
        foreach( _dir IN LISTS HDF5_INCLUDE_DIR )
          if( EXISTS "${_dir}/H5pubconf.h" )
            file( STRINGS "${_dir}/H5pubconf.h"
              HDF5_HAVE_PARALLEL_DEFINE
              REGEX "HAVE_PARALLEL 1" )
            if( HDF5_HAVE_PARALLEL_DEFINE )
              set( HDF5_IS_PARALLEL TRUE )
            endif()
          endif()
          set(HDF5_IS_PARALLEL ${HDF5_IS_PARALLEL} CACHE BOOL
            "HDF5 library compiled with parallel IO support" )
          mark_as_advanced( HDF5_IS_PARALLEL )
        endforeach()
        SET( HDF5_FOUND YES )
      ELSE (HDF5_INCLUDE_DIR AND HDF5_BASE_LIBRARY)
        set( HDF5_FOUND NO )
        message("finding HDF5 failed, please try to set the var HDF5_ROOT")
      ENDIF(HDF5_INCLUDE_DIR AND HDF5_BASE_LIBRARY)
    ENDIF (NOT HDF5_FOUND)

    #now we create fake targets to be used
    include(${HDF5_ROOT}/share/cmake/hdf5/hdf5-targets.cmake OPTIONAL)

  endif(EXISTS "${HDF5_ROOT}/share/cmake/hdf5/hdf5-config.cmake")
endif (HDF5_FOUND)

message (STATUS "---   HDF5 Configuration ::")
message (STATUS "        IS_PARALLEL  : ${HDF5_IS_PARALLEL}")
message (STATUS "        INCLUDES     : ${HDF5_INCLUDES}")
message (STATUS "        LIBRARIES    : ${HDF5_LIBRARIES}")

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (
  HDF5 "HDF5 not found, check environment variables HDF5_ROOT"
  HDF5_INCLUDES
  HDF5_LIBRARIES
  )

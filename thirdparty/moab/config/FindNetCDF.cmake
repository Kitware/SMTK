#
# Find NetCDF include directories and libraries
#
# NETCDF_INCLUDES            - list of include paths to find netcdf.h
# NETCDF_LIBRARIES           - list of libraries to link against when using NetCDF
# NETCDF_FOUND               - Do not attempt to use NetCDF if "no", "0", or undefined.

set (NETCDF_DIR "" CACHE PATH "Path to search for NetCDF header and library files" )
set (NETCDF_FOUND NO CACHE INTERNAL "Found NetCDF components successfully." )

if (EXISTS "${NETCDF_DIR}/lib/pkgconfig/netcdf.pc")
  execute_process(COMMAND pkg-config --cflags-only-I "${NETCDF_DIR}/lib/pkgconfig/netcdf.pc" OUTPUT_VARIABLE NETCDF_INCLUDES)
  execute_process(COMMAND pkg-config --libs-only-L "${NETCDF_DIR}/lib/pkgconfig/netcdf.pc" OUTPUT_VARIABLE NETCDF_LDLAGS)
  execute_process(COMMAND pkg-config --libs-only-l "${NETCDF_DIR}/lib/pkgconfig/netcdf.pc" OUTPUT_VARIABLE NETCDF_LIBRARY)
  execute_process(COMMAND pkg-config --libs "${NETCDF_DIR}/lib/pkgconfig/netcdf.pc" OUTPUT_VARIABLE NETCDF_LIBS)

  IF (NOT NETCDF_FOUND)
    if ( NETCDF_INCLUDES AND NETCDF_LIBS )
      string(STRIP "${NETCDF_INCLUDES}" NETCDF_INCLUDES)
      string(STRIP "${NETCDF_LIBS}" NETCDF_LIBS)
      include (ResolveCompilerPaths)
      RESOLVE_INCLUDES(NETCDF_INCLUDES "${NETCDF_INCLUDES}")
      #RESOLVE_LIBRARIES(NETCDF_LIBRARIES "${NETCDF_LIBS}")
      SET(NETCDF_LIBRARIES ${NETCDF_LIBS})
      SET(NETCDF_FOUND YES )
    else ( NETCDF_INCLUDES AND NETCDF_LIBS )
      set( NETCDF_FOUND NO )
    endif ( NETCDF_INCLUDES AND NETCDF_LIBS )
  ENDIF (NOT NETCDF_FOUND)

else (EXISTS "${NETCDF_DIR}/lib/pkgconfig/netcdf.pc")

  find_path( NETCDF_INCLUDE_DIR netcdf.h
    HINTS ${NETCDF_DIR}
    ${NETCDF_DIR}/include
    ENV CPLUS_INCLUDE_PATH
    NO_DEFAULT_PATH
  )

find_library( NETCDF_C_LIBRARY
    NAMES libnetcdf.a netcdf
    HINTS ${NETCDF_DIR}
    ${NETCDF_DIR}/lib64
    ${NETCDF_DIR}/lib
    NO_DEFAULT_PATH
  )

find_library( NETCDF_CXX_LIBRARY
    NAMES netcdf_c++
    HINTS ${NETCDF_DIR}
    ${NETCDF_DIR}/lib64
    ${NETCDF_DIR}/lib
    NO_DEFAULT_PATH
  )

find_library( NETCDF_FORTRAN_LIBRARY
    NAMES netcdf_g77 netcdff netcdf_ifc netcdf_x86_64
    HINTS ${NETCDF_DIR}
    ${NETCDF_DIR}/lib64
    ${NETCDF_DIR}/lib
    NO_DEFAULT_PATH
  )

  IF (NOT NETCDF_FOUND)
    if ( NETCDF_INCLUDE_DIR AND NETCDF_C_LIBRARY )
      set(NETCDF_FOUND YES )
      set(NETCDF_INCLUDES "${NETCDF_INCLUDE_DIR}")
      set(NETCDF_LIBRARIES ${NETCDF_C_LIBRARY})
      if ( NETCDF_CXX_LIBRARY )
        set(NETCDF_LIBRARIES ${NETCDF_LIBRARIES} ${NETCDF_CXX_LIBRARY})
      endif ( NETCDF_CXX_LIBRARY )
      if ( NETCDF_FORTRAN_LIBRARY )
        set(NETCDF_LIBRARIES ${NETCDF_LIBRARIES} ${NETCDF_FORTRAN_LIBRARY})
      endif ( NETCDF_FORTRAN_LIBRARY )
    else ( NETCDF_INCLUDE_DIR AND NETCDF_C_LIBRARY )
      set( NETCDF_FOUND NO )
      message("finding NetCDF failed, please try to set the var NETCDF_DIR")
    endif ( NETCDF_INCLUDE_DIR AND NETCDF_C_LIBRARY )
  ENDIF (NOT NETCDF_FOUND)

endif (EXISTS "${NETCDF_DIR}/lib/pkgconfig/netcdf.pc")

message (STATUS "---   NetCDF Configuration ::")
message (STATUS "        Found  : ${NETCDF_FOUND}")
message (STATUS "        INCLUDES  : ${NETCDF_INCLUDES}")
message (STATUS "        LIBRARIES : ${NETCDF_LIBRARIES}")

mark_as_advanced(
  NETCDF_DIR
  NETCDF_INCLUDES
  NETCDF_LIBRARIES
)

IF (MOAB_HAVE_MPI)
  set (PNETCDF_DIR "" CACHE PATH "Path to search for parallel NetCDF header and library files" )
  set (PNETCDF_FOUND NO CACHE INTERNAL "Found parallel NetCDF components successfully." )

  find_path( PNETCDF_INCLUDES pnetcdf.h
    ${PNETCDF_DIR}
    ${PNETCDF_DIR}/include
    ENV CPLUS_INCLUDE_PATH
    NO_DEFAULT_PATH
  )

find_library( PNETCDF_LIBRARIES
    NAMES pnetcdf
    HINTS ${PNETCDF_DIR}
    ${PNETCDF_DIR}/lib64
    ${PNETCDF_DIR}/lib
    NO_DEFAULT_PATH
  )

  IF (NOT PNETCDF_FOUND)
    if ( PNETCDF_INCLUDES AND PNETCDF_LIBRARIES )
      set( PNETCDF_FOUND YES )
      message (STATUS "---   PNetCDF Configuration ::")
      message (STATUS "        Found  : ${PNETCDF_FOUND}")
      message (STATUS "        INCLUDES  : ${PNETCDF_INCLUDES}")
      message (STATUS "        LIBRARIES : ${PNETCDF_LIBRARIES}")
    else ( PNETCDF_INCLUDES AND PNETCDF_LIBRARIES )
      set( PNETCDF_FOUND NO )
      message("finding PNetCDF failed, please try to set the var PNETCDF_DIR")
    endif ( PNETCDF_INCLUDES AND PNETCDF_LIBRARIES )
  ENDIF (NOT PNETCDF_FOUND)

  mark_as_advanced(
    PNETCDF_DIR
    PNETCDF_INCLUDES
    PNETCDF_LIBRARIES
  )
ELSE (MOAB_HAVE_MPI)
  message (STATUS "Not configuring with PNetCDF since MPI installation not specified")
ENDIF()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (NetCDF "NetCDF not found, check environment variables NETCDF_DIR"
  NETCDF_DIR NETCDF_INCLUDES NETCDF_LIBRARIES)
find_package_handle_standard_args (PNetCDF "PNetCDF not found, check environment variables PNETCDF_DIR"
  PNETCDF_DIR PNETCDF_INCLUDES PNETCDF_LIBRARIES)


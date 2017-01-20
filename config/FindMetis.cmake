# Find the METIS includes and libraries
#
# METIS is a serial library that implements a variety of algorithms for
# partitioning unstructured graphs, meshes, and for computing fill-reducing orderings of
# sparse matrices. It can be found at:
# 	http://www-users.cs.umn.edu/~karypis/metis/index.html
#
# METIS_INCLUDE_DIR - where to find autopack.h
# METIS_LIBRARIES   - List of fully qualified libraries to link against.
# METIS_FOUND       - Do not attempt to use if "no" or undefined.

set (METIS_DIR "" CACHE PATH "Path to search for Metis header and library files")
set (METIS_FOUND NO CACHE INTERNAL "Found Metis components successfully." )

FIND_LIBRARY(METIS_LIBRARY metis
  HINTS
  ${METIS_DIR}
  ${METIS_DIR}/lib
  ${PARMETIS_DIR}
  ${PARMETIS_DIR}/lib
  )

FIND_PATH(METIS_INCLUDE_DIR metis.h
  HINTS
  ${METIS_DIR}
  ${METIS_DIR}/include
  )

IF (NOT METIS_FOUND)
  if ( METIS_INCLUDE_DIR AND METIS_LIBRARY )
    set( METIS_FOUND YES )
    SET(METIS_INCLUDES ${METIS_INCLUDE_DIR})
    SET(METIS_LIBRARIES ${METIS_LIBRARY})
    message (STATUS "---   Metis Configuration ::")
    message (STATUS "        INCLUDES  : ${METIS_INCLUDES}")
    message (STATUS "        LIBRARIES : ${METIS_LIBRARIES}")
  else ( METIS_INCLUDE_DIR AND METIS_LIBRARY )
    set( METIS_FOUND NO )
    message("finding Metis failed, please try to set the var METIS_DIR")
  endif ( METIS_INCLUDE_DIR AND METIS_LIBRARY )
ENDIF (NOT METIS_FOUND)

mark_as_advanced(
  METIS_DIR
  METIS_INCLUDES
  METIS_LIBRARIES
)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (
  Metis "Metis not found, check environment variables METIS_DIR"
  METIS_INCLUDES
  METIS_LIBRARIES
  )

# Find the PARMETIS includes and libraries
#
# ParMETIS is an MPI-based parallel library that implements a variety of algorithms for
# partitioning unstructured graphs, meshes, and for computing fill-reducing orderings of
# sparse matrices. It can be found at:
# 	http://www-users.cs.umn.edu/~karypis/metis/parmetis/index.html
#
# PARMETIS_INCLUDE_DIR - where to find autopack.h
# PARMETIS_LIBRARIES   - List of fully qualified libraries to link against.
# PARMETIS_FOUND       - Do not attempt to use if "no" or undefined.

set (PARMETIS_DIR "" CACHE PATH "Path to search for ParMetis header and library files")
set (PARMETIS_FOUND NO CACHE INTERNAL "Found ParMetis components successfully.")

FIND_PATH(PARMETIS_INCLUDE_DIR parmetis.h
  HINTS
  ${PARMETIS_DIR}
  ${PARMETIS_DIR}/include
  )

FIND_LIBRARY(PARMETIS_LIBRARY parmetis
  HINTS
  ${PARMETIS_DIR}
  ${PARMETIS_DIR}/lib
  )

IF (NOT PARMETIS_FOUND)
  if ( PARMETIS_INCLUDE_DIR AND PARMETIS_LIBRARY )
    set( PARMETIS_FOUND YES )
    SET(PARMETIS_INCLUDES ${PARMETIS_INCLUDE_DIR})
    SET(PARMETIS_LIBRARIES ${PARMETIS_LIBRARY})
    message (STATUS "---   ParMetis Configuration ::")
    message (STATUS "        INCLUDES  : ${PARMETIS_INCLUDES}")
    message (STATUS "        LIBRARIES : ${PARMETIS_LIBRARIES}")
  else ( PARMETIS_INCLUDE_DIR AND PARMETIS_LIBRARY )
    set( PARMETIS_FOUND NO )
    message("finding ParMetis failed, please try to set the var PARMETIS_DIR")
  endif ( PARMETIS_INCLUDE_DIR AND PARMETIS_LIBRARY )
ENDIF (NOT PARMETIS_FOUND)

mark_as_advanced(
  PARMETIS_DIR
  PARMETIS_INCLUDES
  PARMETIS_LIBRARIES
)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (
  ParMetis "ParMetis not found, check environment variables PARMETIS_DIR"
  PARMETIS_INCLUDES
  PARMETIS_LIBRARIES
  )

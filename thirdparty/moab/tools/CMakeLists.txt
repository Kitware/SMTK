# Documentation
# Optional tools
option ( MOAB_BUILD_MBCONVERT    "Build the MOAB mesh converter tool?"           ON )
option ( MOAB_BUILD_HEXMODOPS    "Build the hexahedral modops tool?"             ON )
option ( MOAB_BUILD_MBCHACO      "Build MOAB Chaco interface?"                  OFF )
option ( MOAB_BUILD_QVDUAL       "Build the qvdual application?"                OFF )
option ( MOAB_BUILD_MBSIZE       "Build the mbsize tool?"                        ON )
option ( MOAB_BUILD_MBMEM        "Build the mbmem tool?"                         ON )
option ( MOAB_BUILD_MBSKIN       "Build the mbskin tool?"                        ON )
option ( MOAB_BUILD_MBDEPTH      "Build the mbdepth tool?"                       ON )
option ( MOAB_BUILD_MBTAGPROP    "Build the mbtagprop tool?"                     ON )
option ( MOAB_BUILD_MBGSETS      "Build the mbgsets tool?"                       ON )
option ( MOAB_BUILD_SPHEREDECOMP "Build the sphere decomposition tool?"          ON )
option ( MOAB_BUILD_MBSURFPLOT   "Build the mbsurfplot application?"             ON )
option ( MOAB_BUILD_MBPART       "Build the mbpart (Zoltan-based) partitioner?" OFF )
option ( MOAB_BUILD_DAGMC        "Build the dagmc tool?"                         ON )
option ( MOAB_BUILD_MBCOUPLER    "Build the mesh coupler tool?"                  ON )

if ( MOAB_HAVE_MPI AND MPI_FOUND )
  add_subdirectory( refiner )
endif ( )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_BINARY_DIR}/src/parallel
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/parallel
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# MBSIZE
if ( MOAB_BUILD_MBSIZE )
  add_executable( mbsize size.cpp measure.cpp)
  set_source_files_properties( size.cpp measure.cpp COMPILE_FLAGS "${MOAB_DEFINES}" )
  target_link_libraries( mbsize MOAB )
  set( MOAB_HAVE_MBSIZE 1 )
  install(TARGETS mbsize RUNTIME DESTINATION bin
                         LIBRARY DESTINATION lib
                         ARCHIVE DESTINATION lib )
endif ( )

# MBMEM
if ( MOAB_BUILD_MBMEM )
  add_executable( mbmem mbmem.cpp)
  set_source_files_properties( mbmem.cpp COMPILE_FLAGS "${MOAB_DEFINES}" )
  target_link_libraries( mbmem MOAB )
  set( MOAB_HAVE_MBMEM 1 )
  install(TARGETS mbmem RUNTIME DESTINATION bin
                         LIBRARY DESTINATION lib
                         ARCHIVE DESTINATION lib )
endif ( )

# MBCONVERT
if ( MOAB_BUILD_MBCONVERT )
  add_executable( mbconvert convert.cpp)
  set_source_files_properties( convert.cpp COMPILE_FLAGS "${MOAB_DEFINES}" )
  target_link_libraries( mbconvert MOAB )
  set( MOAB_HAVE_MBCONVERT 1 )
  install(TARGETS mbconvert RUNTIME DESTINATION bin
                            LIBRARY DESTINATION lib
                            ARCHIVE DESTINATION lib )
endif ( )

# MBDEPTH
if ( MOAB_BUILD_MBDEPTH )
  add_executable( mbdepth depth.cpp)
  set_source_files_properties( depth.cpp COMPILE_FLAGS "${MOAB_DEFINES}" )
  target_link_libraries( mbdepth MOAB )
  set( MOAB_HAVE_MBDEPTH 1 )
  install(TARGETS mbdepth RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib )
endif ( )

# MBSKIN
if ( MOAB_BUILD_MBSKIN )
  add_executable( mbskin skin.cpp)
  set_source_files_properties( skin.cpp COMPILE_FLAGS "${MOAB_DEFINES}" )
  target_link_libraries( mbskin MOAB )
  set( MOAB_HAVE_MBSKIN 1 )
  install(TARGETS mbskin RUNTIME DESTINATION bin
                         LIBRARY DESTINATION lib
                         ARCHIVE DESTINATION lib )
endif ( )

# MBSURFPLOT
if ( MOAB_BUILD_MBSURFPLOT )
  add_executable( mbsurfplot surfplot.cpp )
  set_source_files_properties( surfplot.cpp COMPILE_FLAGS "${MOAB_DEFINES}" )
  target_link_libraries( mbsurfplot MOAB )
  set( MOAB_HAVE_MBSURFPLOT 1 )
  install(TARGETS mbsurfplot RUNTIME DESTINATION bin
                             LIBRARY DESTINATION lib
                             ARCHIVE DESTINATION lib )
endif ( )

# MBTAGPROP
if ( MOAB_BUILD_MBTAGPROP )
  add_executable( mbtagprop parse.cpp propagate_tags.cpp  )
  set_source_files_properties( parse.cpp propagate_tags.cpp COMPILE_FLAGS "${MOAB_DEFINES}" )
  target_link_libraries( mbtagprop MOAB )
  set( MOAB_HAVE_MBTAGPROP 1 )
  install(TARGETS mbtagprop RUNTIME DESTINATION bin
                            LIBRARY DESTINATION lib
                            ARCHIVE DESTINATION lib )
endif ( )

# MBGSETS
if ( MOAB_BUILD_MBGSETS )
  add_executable( mbgsets gsets.cc )
  set_source_files_properties( gsets.cc COMPILE_FLAGS "${MOAB_DEFINES}" )
  target_link_libraries( mbgsets MOAB )
  set( MOAB_HAVE_MBGSETS 1 )
  install(TARGETS mbgsets RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib )
endif( )

# CGM + DagMC
if ( MOAB_BUILD_DAGMC )
  add_subdirectory( dagmc )
endif( )

# Zoltan
if ( MOAB_BUILD_MBPART AND MOAB_HAVE_ZOLTAN )
  # Build the mbpart executable
  add_executable( mbpart mbpart.cpp )
  include_directories( ${ZOLTAN_INCLUDES} )
  set_source_files_properties( mbpart.cpp COMPILE_FLAGS "${MOAB_DEFINES}" )
  target_link_libraries( mbpart MOAB ${ZOLTAN_LIBRARIES} )
  set( MOAB_HAVE_MBPART 1 )
  install(TARGETS mbpart  RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib )
endif( MOAB_BUILD_MBPART AND MOAB_HAVE_ZOLTAN )

# QVDual
if ( MOAB_BUILD_QVDUAL )
  add_subdirectory( qvdual )
endif( )

# MBCoupler
if ( ENABLE_IMESH AND MOAB_HAVE_MPI AND MPI_FOUND AND MOAB_BUILD_MBCOUPLER )
  add_subdirectory( mbcoupler )
endif ( )

# hexmodops
if( MOAB_BUILD_HEXMODOPS )
  add_executable( hexmodops makeops.cpp )
  target_link_libraries( hexmodops MOAB )
  set( MOAB_HAVE_HEXMODOPS 1 )
  install(TARGETS hexmodops RUNTIME DESTINATION bin
                            LIBRARY DESTINATION lib
                            ARCHIVE DESTINATION lib )
endif()

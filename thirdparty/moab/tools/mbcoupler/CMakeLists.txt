include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/parallel
    ${CMAKE_SOURCE_DIR}/src/moab/point_locater/lotte
    ${CMAKE_SOURCE_DIR}/itaps
    ${CMAKE_SOURCE_DIR}/itaps/imesh
    ${CMAKE_BINARY_DIR}/itaps/imesh
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/parallel
)

set( MBCOUPLER_SRCS Coupler.cpp
                    ElemUtil.cpp )

set( MBCOUPLER_INCLUDE_SRCS Coupler.hpp
                            ElemUtil.hpp )

set_source_files_properties( ${MBCOUPLER_SRCS}
  COMPILE_FLAGS "${MOAB_DEFINES} -DIS_BUILDING_MB"
)

add_library( mbcoupler
  ${MBCOUPLER_SRCS}
  ${MBCOUPLER_INCLUDE_SRCS} )

target_link_libraries( mbcoupler iMesh MOAB )
if ( MOAB_HAVE_MPI )
  target_link_libraries( mbcoupler ${MPI_LIBRARIES} )
endif ( )

if ( MOAB_HAVE_NETCDF )
  set(netcdf_progs update_coords)
endif()

set( MOAB_HAVE_MBCOUPLER 1 )
install(TARGETS mbcoupler RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib )
install(FILES ${MBCOUPLER_INCLUDE_SRCS} DESTINATION include)


if ( MOAB_HAVE_MPI )
  # Exercise the coupler and not just findpt:
  add_executable( mbcoupler_test mbcoupler_test.cpp )
  set_source_files_properties( mbcoupler_test.cpp
    COMPILE_FLAGS "${MOAB_DEFINES} -DMESHDIR=${MOAB_ABSSRC_DIR}/MeshFiles/unittest" )
  target_link_libraries( mbcoupler_test mbcoupler iMesh MOAB ${MPI_LIBRARIES} )
  add_test( TestMBCoupler ${EXECUTABLE_OUTPUT_PATH}/mbcoupler_test )

  add_executable( ssn_test ssn_test.cpp )
  set_source_files_properties( ssn_test.cpp
    COMPILE_FLAGS "${MOAB_DEFINES} -DTEST -DMESHDIR=${MOAB_ABSSRC_DIR}/MeshFiles/unittest" )
  target_link_libraries( ssn_test mbcoupler iMesh MOAB ${MPI_LIBRARIES} )
  add_test( TestSSN ${EXECUTABLE_OUTPUT_PATH}/ssn_test )
endif ( MOAB_HAVE_MPI )

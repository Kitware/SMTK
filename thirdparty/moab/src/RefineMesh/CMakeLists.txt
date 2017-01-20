if( MOAB_HAVE_HDF5 )
  include_directories(${HDF5_INCLUDES})
  set( MOAB_HDF_LIBS ${HDF5_LIBRARIES} )
endif()
if( MOAB_HAVE_NETCDF )
  include_directories(${NETCDF_INCLUDES})
  if ( MOAB_HAVE_MPI )
    set( MOAB_NETCDF_LIBS ${PNETCDF_LIBRARIES} ${NETCDF_LIBRARIES} )
  else ( MOAB_HAVE_MPI )
    set( MOAB_NETCDF_LIBS ${NETCDF_LIBRARIES} )
  endif ( MOAB_HAVE_MPI )
endif()

set(MOAB_NR_LIB_SRCS NestedRefine.cpp)
set(MOAB_NR_INC_HDRS moab/NestedRefine.hpp moab/Templates.hpp)

add_library( moab_refine_mesh OBJECT ${MOAB_NR_LIB_SRCS} ${MOAB_NR_INC_HDRS} )
# Set position-independent flag so that the object files are embedded properly.
set_target_properties(moab_refine_mesh PROPERTIES POSITION_INDEPENDENT_CODE True)
set_target_properties( moab_refine_mesh
                       PROPERTIES COMPILE_FLAGS "-DIS_BUILDING_MB ${MOAB_DEFINES}")

include(AutoconfHeader)
moab_install_headers(${MOAB_NR_INC_HDRS})

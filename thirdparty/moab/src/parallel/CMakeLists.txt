set( moab_PARALLEL_SRCS
  ParallelComm.cpp
  ProcConfig.cpp
  ParallelData.cpp
  ParallelMergeMesh.cpp
  ReadParallel.hpp      ReadParallel.cpp
  SharedSetData.hpp     SharedSetData.cpp
  gs.cpp
)

#generate the moab_mpi_config.h file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/moab_mpi_config.h.in "${CMAKE_CURRENT_BINARY_DIR}/moab_mpi_config.h" @ONLY)

set(moab_PARALLEL_INSTALL_HDRS moab/gs.hpp
                               moab_mpi.h
                               moab/ParallelComm.hpp
                               moab/ParallelMergeMesh.hpp
                               moab/ProcConfig.hpp
                               moab/ParallelData.hpp
                               MBParallelConventions.h )

set( MOAB_GENERATED_INSTALL_PHEADERS
     ${CMAKE_CURRENT_BINARY_DIR}/moab_mpi_config.h )

include_directories(
    ${MOAB_BINARY_DIR}/src
    ${MOAB_SOURCE_DIR}/src
    ${MOAB_SOURCE_DIR}/src/parallel
    ${CMAKE_CURRENT_BINARY_DIR}
  )

if ( MOAB_HAVE_HDF5 AND MOAB_HAVE_HDF5_PARALLEL )
  set( moab_PARALLEL_SRCS ${moab_PARALLEL_SRCS}
                          WriteHDF5Parallel.cpp 
                          WriteHDF5Parallel.hpp )
endif()

if ( MOAB_HAVE_HDF5 )
  include_directories(
    ${MOAB_SOURCE_DIR}/mhdf/include
  )
endif ( MOAB_HAVE_HDF5 )

set_source_files_properties( ${moab_PARALLEL_SRCS}
  COMPILE_FLAGS "-DSRCDIR=\"${MOAB_SOURCE_DIR}\""
)

add_library(
  moabpar
  OBJECT ${moab_PARALLEL_INSTALL_HDRS} ${MOAB_GENERATED_INSTALL_PHEADERS} ${moab_PARALLEL_SRCS})
if (BUILD_SHARED_LIBS)
  set(shared_define "-DIS_BUILDING_MB_SHARED")
endif ()
set_target_properties(
  moabpar
  PROPERTIES
  COMPILE_FLAGS "${shared_define} -DIS_BUILDING_MB ${MOAB_DEFINES}"
  )

include(AutoconfHeader)
moab_install_headers(${moab_PARALLEL_INSTALL_HDRS})
foreach (header ${MOAB_GENERATED_INSTALL_PHEADERS})
    STRING(REGEX REPLACE "^${CMAKE_CURRENT_BINARY_DIR}/" "" relative ${header})
    string(REGEX MATCH "(.*)/" DIR ${relative})
    install(
      FILES ${header}
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${DIR}
      )
endforeach()

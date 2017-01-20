
set ( MOAB_IMESH_SRCS
          iMesh_MOAB.hpp iMesh_MOAB.cpp
          MBIter.hpp )
set ( MOAB_IMESH_LIB_INCLUDES
          iMesh.h
          iMesh_extensions.h
          iMesh_f.h
          iMesh_protos.h
          MBiMesh.hpp )

if ( MOAB_HAVE_MPI )
  include_directories( ${CMAKE_SOURCE_DIR}/src/parallel
                       ${CMAKE_BINARY_DIR}/src/parallel)
  set(MOAB_IMESH_SRCS ${MOAB_IMESH_SRCS}
                      iMeshP_MOAB.cpp)
  set(MOAB_IMESH_LIB_INCLUDES
          ${MOAB_IMESH_LIB_INCLUDES}
          iMeshP.h
          iMeshP_protos.h )
endif()

if ( MOAB_HAVE_HDF5 AND HDF5_FOUND )
  include_directories( ${HDF5_INCLUDE_DIRECTORIES}
                       ${CMAKE_SOURCE_DIR}/src/io/mhdf/include )
endif( )

set_source_files_properties(
  ${MOAB_IMESH_SRCS} COMPILE_FLAGS "${MOAB_DEFINES}"
  )

add_library(
  iMesh
  ${MOAB_IMESH_SRCS}
  ${MOAB_IMESH_LIB_INCLUDES}
  )
include(GenerateExportHeader)
generate_export_header(iMesh)
list(APPEND MOAB_IMESH_LIB_INCLUDES
  ${CMAKE_CURRENT_BINARY_DIR}/imesh_export.h)

if (MOAB_USE_MPI)
  target_include_directories(iMesh
    PUBLIC
      ${MPI_INCLUDE_PATH})
  target_link_libraries(iMesh
    PUBLIC
      ${MPI_LIBRARIES})
endif ()

target_link_libraries( iMesh PUBLIC MOAB )
target_include_directories(iMesh PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
  "$<INSTALL_INTERFACE:include>")

set_target_properties(
  iMesh
  PROPERTIES
  VERSION ${MOAB_VERSION}
  SOVERSION ${MOAB_VERSION_MAJOR}
  )

install(
  TARGETS iMesh
  EXPORT SMTK
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
set( MOAB_INSTALL_TARGETS ${MOAB_INSTALL_TARGETS} iMesh)

install(
  FILES ${MOAB_IMESH_LIB_INCLUDES}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

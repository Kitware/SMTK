set( FBiGEOM_SOURCE FBiGeom_MOAB.hpp FBiGeom_MOAB.cpp )
set(
  FBiGEOM_INCLUDE_HEADERS
  FBiGeom.h
  FBiGeom_protos.h
  FBiGeom_f.h
  )

add_library(
  FBiGeomMOAB
  ${FBiGEOM_SOURCE}
  ${FBiGEOM_INCLUDE_HEADERS}
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/itaps
  ${CMAKE_BINARY_DIR}/itaps
  ${CMAKE_SOURCE_DIR}/itaps/imesh
  ${CMAKE_BINARY_DIR}/itaps/imesh
 )

if ( MOAB_HAVE_MPI )
  include_directories( ${MOAB_SOURCE_DIR}/src/parallel
                       ${MOAB_BINARY_DIR}/src/parallel)
endif ()

target_link_libraries( FBiGeomMOAB MOAB iMesh )
target_include_directories(FBiGeomMOAB
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:include>")

set_target_properties(
  FBiGeomMOAB
  PROPERTIES
  VERSION ${MOAB_VERSION}
  SOVERSION ${MOAB_VERSION_MAJOR}
  )

moab_install_headers(${FBiGEOM_INCLUDE_HEADERS})
install(
  TARGETS FBiGeomMOAB
  EXPORT MOABTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

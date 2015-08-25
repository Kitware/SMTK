set ( MOAB_REFINER_SRCS
  EdgeSizeEvaluator.cpp
  EdgeSizeSimpleImplicit.cpp
  EntityRefiner.cpp
  MeshOutputFunctor.cpp
  MeshRefiner.cpp
  ProcessSet.cpp
  RefinerTagManager.cpp
  SimplexTemplateRefiner.cpp
  SimplexTemplateTagAssigner.cpp
  SplitVertices.cpp
  )

include_directories(
    ${MOAB_BINARY_DIR}
    ${MOAB_BINARY_DIR}/src
    ${MOAB_BINARY_DIR}/src/parallel
    ${MOAB_SOURCE_DIR}/src
    ${MOAB_SOURCE_DIR}/src/parallel
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_source_files_properties(
  ${MOAB_REFINER_SRCS}
  COMPILE_FLAGS "${MOAB_DEFINES}"
)

add_library( MOABrefiner
  ${MOAB_REFINER_SRCS}
  )
target_link_libraries( MOABrefiner MOAB )
include(GenerateExportHeader)
generate_export_header(MOABrefiner)
if ( MOAB_HAVE_MPI )
  target_link_libraries( MOABrefiner ${MPI_LIBRARIES} )
endif ( MOAB_HAVE_MPI )

enable_testing()

add_executable( test_mesh_refiner test_mesh_refiner.cpp )
set_source_files_properties( test_mesh_refiner.cpp
  COMPILE_FLAGS "-DTEST ${MOAB_DEFINES}" )
target_link_libraries( test_mesh_refiner MOAB MOABrefiner )

add_test( TestMeshRefiner
  ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 4 ${MPIEXEC_PREFLAGS}
  ${EXECUTABLE_OUTPUT_PATH}/test_mesh_refiner ${MPIEXEC_POSTFLAGS}
  ${MOAB_SOURCE_DIR}/tools/refiner/fourVolsBare.cub )


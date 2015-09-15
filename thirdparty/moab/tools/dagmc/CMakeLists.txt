include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src )

add_library( dagmc DagMC.hpp DagMC.cpp )
include(GenerateExportHeader)
generate_export_header(dagmc)
target_link_libraries( dagmc  MOAB)
install(TARGETS dagmc EXPORT SMTK
                      RUNTIME DESTINATION bin
                      LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib )
set( MOAB_INSTALL_TARGETS ${MOAB_INSTALL_TARGETS} dagmc)
install( FILES DagMC.hpp ${CMAKE_CURRENT_BINARY_DIR}/dagmc_export.h DESTINATION include )



add_executable( dagmc_preproc dagmc_preproc.cpp
                              obb_analysis.cpp
                              dagmc_preproc.hpp )
target_link_libraries( dagmc_preproc MOAB dagmc )
set( MOAB_HAVE_DAGMC 1 )
install(TARGETS dagmc_preproc RUNTIME DESTINATION bin
                              LIBRARY DESTINATION lib
                              ARCHIVE DESTINATION lib )

if ( MOAB_ENABLE_TESTING )
  add_executable( test_geom test_geom.cc )
  set_target_properties( test_geom PROPERTIES COMPILE_FLAGS "${MOAB_DEFINES} -DTEST" )
  target_link_libraries( test_geom dagmc MOAB)
  if (MOAB_USE_CGM)
    target_link_libraries( test_geom ${CGM_LIBRARIES} )
  endif ()
  add_test( test_geom ${EXECUTABLE_OUTPUT_PATH}/test_geom )
endif()

if ( MOAB_HAVE_CGM )
  # Find CGM directory
  find_package( CGM )

  if ( CGM_FOUND )
    include_directories( ${CGM_INCLUDE_DIRECTORIES} )
  endif( )

  target_link_libraries( dagmc ${CGM_LIBRARIES} )
  option ( MOAB_BUILD_CGM2MOAB    "Build cgm2moab?"  OFF )
  if(MOAB_BUILD_CGM2MOAB)
    add_executable( cgm2moab dagmc_preproc.cpp
                             obb_analysis.cpp
                             dagmc_preproc.hpp )
    target_link_libraries( cgm2moab
                           dagmc
                           ${CGM_LIBRARIES} )
    install(TARGETS cgm2moab RUNTIME DESTINATION bin
                             LIBRARY DESTINATION lib
                             ARCHIVE DESTINATION lib )
  endif()
endif( )

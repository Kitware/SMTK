# Declare unit tests Usage:
#
# smtk_unit_tests(
#   LABEL <prefix for all unit tests>
#   SOURCES <test_source_list>
#   SOURCES_REQUIRE_DATA <test_sources_that_require_SMTK_DATA_DIR>
#   EXTRA_SOURCES <helper_source_files>
#   LIBRARIES <dependent_library_list>
#   )
function(smtk_unit_tests)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs LABEL SOURCES SOURCES_REQUIRE_DATA EXTRA_SOURCES LIBRARIES)
  cmake_parse_arguments(SMTK_ut
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN}
    )

  set(have_testing_data OFF)
  set(using_hdf OFF)
  if(MOAB_USE_HDF)
    set(using_hdf ON)
  elseif(ENABLE_HDF5)
    set(using_hdf ON)
  endif()

  if (SMTK_DATA_DIR)
    if (NOT EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
      message(WARNING
	"SMTK_DATA_DIR has been set to invalid location \"${SMTK_DATA_DIR}\".")
    elseif(SMTK_ENABLE_TESTING AND NOT using_hdf AND NOT ENABLE_HDF5)
      message(WARNING
	"SMTK_DATA_DIR has been set, but hdf5 is not enabled. Skipping tests that use data.")
    endif ()
  endif()

  if (SMTK_DATA_DIR
      AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker
      AND using_hdf)
    #we check moab for hdf support since that is the file format
    #for all our test data
    set(have_testing_data ON)
    list(APPEND SMTK_ut_SOURCES ${SMTK_ut_SOURCES_REQUIRE_DATA})
  endif()

  list(LENGTH SMTK_ut_SOURCES num_sources)
  if(NOT ${num_sources})
    #no sources don't make a target
    return()
  endif()

  if (SMTK_ENABLE_TESTING)
    smtk_get_kit_name(kit)
    #we use UnitTests_ so that it is an unique key to exclude from coverage
    set(test_prog UnitTests_${kit})

    create_test_sourcelist(TestSources ${test_prog}.cxx ${SMTK_ut_SOURCES})
    add_executable(${test_prog} ${TestSources} ${SMTK_ut_EXTRA_SOURCES})

    target_link_libraries(${test_prog} LINK_PRIVATE ${SMTK_ut_LIBRARIES})
    target_include_directories(${test_prog}
        PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${MOAB_INCLUDE_DIRS}
        ${VTK_INCLUDE_DIRS}
        )

    if(have_testing_data)
      target_compile_definitions(${test_prog} PRIVATE "SMTK_DATA_DIR=\"${SMTK_DATA_DIR}\"")
      target_compile_definitions(${test_prog} PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")
    endif()

    foreach (test ${SMTK_ut_SOURCES})
      get_filename_component(tname ${test} NAME_WE)
      add_test(NAME ${tname}
        COMMAND ${test_prog} ${tname}
        )
      set_tests_properties(${tname} PROPERTIES TIMEOUT 120)
      if(SMTK_ut_LABEL)
        set_tests_properties(${tname} PROPERTIES LABELS ${SMTK_ut_LABEL})
      endif()
    endforeach(test)
  endif (SMTK_ENABLE_TESTING)

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
  list(APPEND unit_tests

       )
endif()


endfunction(smtk_unit_tests)

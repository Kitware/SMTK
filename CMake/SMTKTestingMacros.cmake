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
  set(multiValueArgs LABEL SOURCES SOURCES_SERIAL SOURCES_REQUIRE_DATA SOURCES_SERIAL_REQUIRE_DATA EXTRA_SOURCES LIBRARIES)
  cmake_parse_arguments(SMTK_ut
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN}
    )

  set(have_testing_data OFF)
  if (SMTK_DATA_DIR)
    set(have_testing_data ON)
    list(APPEND SMTK_ut_SOURCES ${SMTK_ut_SOURCES_REQUIRE_DATA})
    list(APPEND SMTK_ut_SOURCES_SERIAL ${SMTK_ut_SOURCES_SERIAL_REQUIRE_DATA})
  endif()
  list(APPEND SMTK_ut_SOURCES ${SMTK_ut_SOURCES_SERIAL})

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

    target_compile_definitions(${test_prog} PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")
    if(have_testing_data)
      target_compile_definitions(${test_prog} PRIVATE "SMTK_DATA_DIR=\"${SMTK_DATA_DIR}\"")
    endif()

    foreach (test ${SMTK_ut_SOURCES})
      get_filename_component(tname ${test} NAME_WE)
      add_test(NAME ${tname}
        COMMAND ${test_prog} ${tname} ${${tname}_EXTRA_ARGUMENTS}
        )
      set_tests_properties(${tname} PROPERTIES TIMEOUT 120)
      if(SMTK_ut_LABEL)
        set_tests_properties(${tname} PROPERTIES LABELS "${SMTK_ut_LABEL}")
      endif()
      # If we build python wrappings, some tests will need their PATH and PYTHONPATH
      # set in order to be able to import and run python codef from C++:
      if (SMTK_ENABLE_PYTHON_WRAPPING)
        set_property(TEST ${tname} APPEND PROPERTY ENVIRONMENT "PYTHONPATH=${smtk_pythonpath_env}")
      endif()
    endforeach()

    foreach (test ${SMTK_ut_SOURCES_SERIAL})
      get_filename_component(tname ${test} NAME_WE)
      set_tests_properties(${tname} PROPERTIES RUN_SERIAL TRUE)
      # If we build python wrappings, some tests will need their PATH and PYTHONPATH
      # set in order to be able to import and run python codef from C++:
      if (SMTK_ENABLE_PYTHON_WRAPPING)
        set_property(TEST ${tname} APPEND PROPERTY ENVIRONMENT "PYTHONPATH=${smtk_pythonpath_env}")
      endif()
    endforeach()

  endif ()
endfunction()

# Add tests for code that is expected to fail to build.
#
# For each <test_source_file>, the compiler will be invoked <number_of_failures> times.
# Each time, the macro SMTK_FAILURE_INDEX will be defined as a different integer,
# starting with 1 and ending with <number_of_failures>.
# Thus, the same source code can validate that failure occurs in several different ways.
#
# This macro was inspired by
#   https://stackoverflow.com/questions/30155619/expected-build-failure-tests-in-cmake
#
# smtk_build_failure_tests(
#   LABEL <prefix for all build-failure tests>
#   TESTS <test_source_file> <number_of_failures> [<test_source_file> <number_of_failures> ...]
#   LIBRARIES <dependent_library_list>
# )
function(smtk_build_failure_tests)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs LABEL TESTS LIBRARIES)
  cmake_parse_arguments(SMTK_bft
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN}
  )

  list(LENGTH SMTK_bft_TESTS num_entries)
  if(NOT ${num_entries})
    return()
  endif()

  if (SMTK_ENABLE_TESTING)
    smtk_get_kit_name(kit)
    set(test_prog BuildFailure_${kit})

    math(EXPR num_sources "${num_entries} / 2")
    foreach(source_idx RANGE 0 ${num_sources} 2)
      math(EXPR idx_file "2 * ${source_idx}")
      math(EXPR idx_count "2 * ${source_idx} + 1")
      list(GET SMTK_bft_TESTS ${idx_file} test_src)
      list(GET SMTK_bft_TESTS ${idx_count} test_count)
      math(EXPR trange "${test_count} - 1")
      get_filename_component(thandle "${test_src}" NAME_WE)
      foreach(attempt RANGE 0 ${trange})
        set(tname "${test_prog}_${thandle}_${attempt}")
        add_executable(${tname} ${test_src})
        set_target_properties(${tname} PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)
        target_link_libraries(${tname} LINK_PRIVATE ${SMTK_bft_LIBRARIES})
        target_compile_definitions(${tname} PRIVATE "-DSMTK_FAILURE_INDEX=${attempt}")
        target_include_directories(${tname}
          PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}
            ${MOAB_INCLUDE_DIRS}
            ${VTK_INCLUDE_DIRS}
        )
        add_test(
          NAME ${tname}
          COMMAND ${CMAKE_COMMAND} --build . --target ${tname} --config $<CONFIGURATION>
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        set_tests_properties(${tname} PROPERTIES TIMEOUT 120 WILL_FAIL TRUE)
        if(SMTK_bft_LABEL)
          set_tests_properties(${tname} PROPERTIES LABELS "${SMTK_bft_LABEL}")
        endif()
      endforeach() # attempt
    endforeach() # source_idx
  endif ()
endfunction()

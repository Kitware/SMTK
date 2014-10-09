# Declare unit tests Usage:
#
# smtk_unit_tests(
#   LABEL <prefix for all unit tests>
#   SOURCES <test_source_list>
#   EXTRA_SOURCES <helper_source_files>
#   LIBRARIES <dependent_library_list>
#   )
function(smtk_unit_tests)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs LABEL SOURCES EXTRA_SOURCES LIBRARIES)
  cmake_parse_arguments(SMTK_ut
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN}
    )

  if (SMTK_ENABLE_TESTING)
    smtk_get_kit_name(kit)
    #we use UnitTests_ so that it is an unique key to exclude from coverage
    set(test_prog UnitTests_${kit})
    create_test_sourcelist(TestSources ${test_prog}.cxx ${SMTK_ut_SOURCES})

    add_executable(${test_prog} ${TestSources} ${SMTK_ut_EXTRA_SOURCES})
    target_link_libraries(${test_prog} LINK_PRIVATE ${SMTK_ut_LIBRARIES})
    target_include_directories(${test_prog} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
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
endfunction(smtk_unit_tests)

set(ioTests
  attributeLibraryTest
  extensibleAttributeIOTest
  fileItemTest
  loggerTest
  loggerThreadTest
)

set(ioDataTests
  groupItemCSVTest
  groupItemDoubleFileTest
  )

  set(groupItemCSVTest_ARGUMENTS
    "${SMTK_DATA_DIR}/attribute/csv/groupItemTestData.csv"
    )
  set(groupItemDoubleFileTest_ARGUMENTS
    "${SMTK_DATA_DIR}/attribute/csv/heater.txt"
  )

foreach (test ${ioTests})
  add_executable(${test}  ${test}.cxx)
  target_link_libraries(${test} smtkCore smtkCoreModelTesting ${Boost_LIBRARIES} Threads::Threads)
  add_test(NAME ${test} COMMAND ${test})
  target_compile_definitions(${test} PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")
  set_tests_properties(${test} PROPERTIES SKIP_RETURN_CODE 125)
endforeach()

if (SMTK_DATA_DIR)
  foreach(test ${ioDataTests})
    add_executable(${test} ${test}.cxx)
    target_link_libraries(${test}
      PUBLIC
        smtkCore
        ${Boost_LIBRARIES}
        ${${test}_EXTRA_LIBRARIES}
    )
    add_test(
      NAME ${test}
      COMMAND ${test} ${${test}_ARGUMENTS}
    )
    set_tests_properties(${test} PROPERTIES SKIP_RETURN_CODE 125)
  endforeach()
endif()

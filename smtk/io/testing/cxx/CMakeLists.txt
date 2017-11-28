set(ioTests
  extensibleAttributeIOTest
  fileItemTest
  loggerTest
  ResourceSetTest
  unitSaveLoadJSON
)

set(ioDataTests
  ResourceSetWriterTest
  ResourceSetReaderTest
)

set(ResourceSetWriterTest_ARGUMENTS
  ${SMTK_DATA_DIR}/attribute/attribute_collection/Basic2DFluid.sbt
  ${SMTK_DATA_DIR}/attribute/attribute_collection/copyAttributeTest.sbi
  writer_output.crf
)

set(ResourceSetReaderTest_ARGUMENTS
  ${SMTK_DATA_DIR}/attribute/attribute_collection/resourceTest/resources.xml
  2
)

foreach (test ${ioTests})
  add_executable(${test}  ${test}.cxx)
  target_link_libraries(${test} smtkCore smtkCoreModelTesting ${Boost_LIBRARIES})
  add_test(NAME ${test} COMMAND ${test})
  target_compile_definitions(${test} PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")
endforeach()

# ResourceSetWriterTest uses input files in SMTKTestData
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
  endforeach()
endif()

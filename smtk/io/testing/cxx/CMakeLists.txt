set(ioTests
  extensibleAttributeIOTest
  loggerTest
  ResourceSetTest
  unitImportExportJSON
)

foreach (test ${ioTests})
  add_executable(${test}  ${test}.cxx)
  target_link_libraries(${test} smtkCore smtkCoreModelTesting ${Boost_LIBRARIES})
  add_test(NAME ${test} COMMAND ${test})
  target_compile_definitions(${test} PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")
endforeach()


# ResourceSetWriterTest uses input files in SMTKTestData
if (SMTK_DATA_DIR)
  set(writer_test ResourceSetWriterTest)
  add_executable(${writer_test}  ${writer_test}.cxx)
  target_link_libraries(${writer_test} smtkCore ${Boost_LIBRARIES})
  add_test(NAME ${writer_test}
    COMMAND ${writer_test}
    ${SMTK_DATA_DIR}/attribute/attribute_system/Basic2DFluid.sbt
    ${SMTK_DATA_DIR}/attribute/attribute_system/copyAttributeTest.sbi
    writer_output.crf
  )

  set(reader_test ResourceSetReaderTest)
  add_executable(${reader_test}  ${reader_test}.cxx)
  target_link_libraries(${reader_test} smtkCore ${Boost_LIBRARIES})
  add_test(NAME ${reader_test}
    COMMAND ${reader_test}
    ${SMTK_DATA_DIR}/attribute/attribute_system/resourceTest/resources.xml
    2
  )
endif()

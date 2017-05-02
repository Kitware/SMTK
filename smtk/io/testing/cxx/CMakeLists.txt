set(ioTests
  extensibleAttributeIOTest
  fileItemTest
  loggerTest
  ResourceSetTest
  unitSaveLoadJSON
)

set(ioDataTests
  readSMTKFile
  ResourceSetWriterTest
  ResourceSetReaderTest
)

set(readSMTKFile_EXTRA_LIBRARIES smtkExodusSession)
set(readSMTKFile_ARGUMENTS
  ${SMTK_DATA_DIR}/model/3d/netcdf/pillbox.smtk
)

set(ResourceSetWriterTest_ARGUMENTS
  ${SMTK_DATA_DIR}/attribute/attribute_system/Basic2DFluid.sbt
  ${SMTK_DATA_DIR}/attribute/attribute_system/copyAttributeTest.sbi
  writer_output.crf
)

set(ResourceSetReaderTest_ARGUMENTS
  ${SMTK_DATA_DIR}/attribute/attribute_system/resourceTest/resources.xml
  2
)

foreach (test ${ioTests})
  add_executable(${test}  ${test}.cxx)
  target_link_libraries(${test} smtkCore smtkCoreModelTesting ${Boost_LIBRARIES})
  add_test(NAME ${test} COMMAND ${test})
  target_compile_definitions(${test} PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")
endforeach()

# ResourceSetWriterTest uses input files in SMTKTestData
if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker)
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

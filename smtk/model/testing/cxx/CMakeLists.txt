################################################################################
# Make a library of some utility functions for tests to use:
################################################################################
add_library(SMTKCoreModelTesting STATIC helpers.cxx)
smtk_public_headers(helpers.h)
target_link_libraries(SMTKCoreModelTesting LINK_PRIVATE SMTKCore)
smtk_install_library(SMTKCoreModelTesting)

################################################################################
# Tools and demos
################################################################################

add_executable(demoReportArrangements demoReportArrangements.cxx)
target_link_libraries(demoReportArrangements SMTKCore SMTKCoreModelTesting)

################################################################################
# Tests
################################################################################
add_executable(unitManager unitManager.cxx)
target_link_libraries(unitManager SMTKCore SMTKCoreModelTesting)
add_test(unitManager ${EXECUTABLE_OUTPUT_PATH}/unitManager)

add_executable(unitOperator unitOperator.cxx)
smtk_operator_xml( "${CMAKE_CURRENT_SOURCE_DIR}/unitOutcomeOperator.sbt" unitOperatorXML)
target_link_libraries(unitOperator SMTKCore SMTKCoreModelTesting)
if (SMTK_BUILD_CGM)
  target_link_libraries(unitOperator cgmSMTK)
endif (SMTK_BUILD_CGM)
add_test(unitOperator ${EXECUTABLE_OUTPUT_PATH}/unitOperator)

add_executable(unitCursor unitCursor.cxx)
target_link_libraries(unitCursor SMTKCore SMTKCoreModelTesting)
add_test(unitCursor ${EXECUTABLE_OUTPUT_PATH}/unitCursor)

add_executable(unitDefaultBridge unitDefaultBridge.cxx)
smtk_operator_xml( "${CMAKE_CURRENT_SOURCE_DIR}/unitForwardingOperator.sbt" unitOperatorXML)
target_link_libraries(unitDefaultBridge SMTKCore SMTKCoreModelTesting)
add_test(unitDefaultBridge ${EXECUTABLE_OUTPUT_PATH}/unitDefaultBridge)

add_executable(unitDescriptivePhrase unitDescriptivePhrase.cxx)
target_link_libraries(unitDescriptivePhrase SMTKCore SMTKCoreModelTesting)

add_executable(unitArrangement unitArrangement.cxx)
target_link_libraries(unitArrangement SMTKCore)
add_test(unitArrangement ${EXECUTABLE_OUTPUT_PATH}/unitArrangement)

add_executable(unitEntity unitEntity.cxx)
target_link_libraries(unitEntity SMTKCore SMTKCoreModelTesting)
add_test(unitEntity ${EXECUTABLE_OUTPUT_PATH}/unitEntity)

add_executable(benchmarkModel benchmarkModel.cxx)
target_link_libraries(benchmarkModel SMTKCore SMTKCoreModelTesting)
#add_test(benchmarkModel ${EXECUTABLE_OUTPUT_PATH}/benchmarkModel)

################################################################################
# Tests that require SMTK_DATA_DIR
################################################################################
if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/ReadMe.mkd)
  add_test(unitDescriptivePhrase ${EXECUTABLE_OUTPUT_PATH}/unitDescriptivePhrase
    "${SMTK_DATA_DIR}/smtk/simple.json")

  add_test(unitImportExportJSON ${EXECUTABLE_OUTPUT_PATH}/unitImportExportJSON
    "${SMTK_DATA_DIR}/smtk/simple.json")
endif()

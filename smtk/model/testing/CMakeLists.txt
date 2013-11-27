################################################################################
# Make a library of some utility functions for tests to use:
################################################################################
add_library(SMTKCoreModelTesting STATIC helpers.cxx)
smtk_public_headers(helpers.h)
target_link_libraries(SMTKCoreModelTesting LINK_PRIVATE SMTKCore)
smtk_install_library(SMTKCoreModelTesting)

################################################################################
# Tests
################################################################################
add_executable(unitStorage unitStorage.cxx)
target_link_libraries(unitStorage SMTKCore SMTKCoreModelTesting)
add_test(unitStorage ${EXECUTABLE_OUTPUT_PATH}/unitStorage)

add_executable(unitImportExportJSON unitImportExportJSON.cxx)
target_link_libraries(unitImportExportJSON SMTKCore)
add_test(unitImportExportJSON ${EXECUTABLE_OUTPUT_PATH}/unitImportExportJSON
  "${CMAKE_CURRENT_SOURCE_DIR}/cube.json")

add_executable(unitArrangement unitArrangement.cxx)
target_link_libraries(unitArrangement SMTKCore)
add_test(unitArrangement ${EXECUTABLE_OUTPUT_PATH}/unitArrangement)

add_executable(unitEntity unitEntity.cxx)
target_link_libraries(unitEntity SMTKCore SMTKCoreModelTesting)
add_test(unitEntity ${EXECUTABLE_OUTPUT_PATH}/unitEntity)

add_executable(benchmarkModel benchmarkModel.cxx)
target_link_libraries(benchmarkModel SMTKCore SMTKCoreModelTesting)
#add_test(benchmarkModel ${EXECUTABLE_OUTPUT_PATH}/benchmarkModel)

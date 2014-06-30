smtk_public_headers(helpers.h)

add_executable(loggerTest loggerTest.cxx)
target_link_libraries(loggerTest SMTKCore)
add_test(loggerTest ${EXECUTABLE_OUTPUT_PATH}/loggerTest)

add_executable(unitUUID unitUUID.cxx)
target_link_libraries(unitUUID SMTKCore)
add_test(unitUUID ${EXECUTABLE_OUTPUT_PATH}/unitUUID)

add_executable(ResourceSetTest ResourceSetTest.cxx)
target_link_libraries(ResourceSetTest SMTKCore)
add_test(ResourceSetTest ${EXECUTABLE_OUTPUT_PATH}/ResourceSetTest)

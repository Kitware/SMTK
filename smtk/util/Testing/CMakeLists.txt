add_executable(loggerTest loggerTest.cxx)
target_link_libraries(loggerTest SMTKCore)
add_test(loggerTest ${EXECUTABLE_OUTPUT_PATH}/loggerTest)

add_executable(unitUUID unitUUID.cxx)
target_link_libraries(unitUUID SMTKCore)
add_test(unitUUID ${EXECUTABLE_OUTPUT_PATH}/unitUUID)

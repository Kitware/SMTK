add_executable(loggerTest loggerTest.cxx)

target_link_libraries(loggerTest SMTKCore)

add_test(loggerTest ${EXECUTABLE_OUTPUT_PATH}/loggerTest)

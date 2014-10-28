add_executable(list-cgm-engines list-cgm-engines.cxx)
target_link_libraries(list-cgm-engines cgmSMTK SMTKCore)
add_test(list-cgm-engines ${EXECUTABLE_OUTPUT_PATH}/list-cgm-engines -test-set-default)

add_executable(test-operators test-operators.cxx)
target_link_libraries(test-operators cgmSMTK SMTKCore)
add_test(test-operators ${EXECUTABLE_OUTPUT_PATH}/test-operators)

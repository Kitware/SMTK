add_executable(list-cgm-engines list-cgm-engines.cxx)
target_link_libraries(list-cgm-engines cgmSMTK SMTKCore)
add_test(list-cgm-engines ${EXECUTABLE_OUTPUT_PATH}/list-cgm-engines -test-set-default)

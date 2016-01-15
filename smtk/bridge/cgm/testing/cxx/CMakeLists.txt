add_executable(list-cgm-engines list-cgm-engines.cxx)
target_link_libraries(list-cgm-engines smtkCGMSession smtkCore)
add_test(NAME list-cgm-engines COMMAND list-cgm-engines -test-set-default)

add_executable(test-operators test-operators.cxx)
target_link_libraries(test-operators smtkCGMSession smtkCore ${Boost_LIBRARIES})
add_test(NAME test-operators COMMAND test-operators)

add_executable(basicAttributeTest basicAttributeTest.cxx)

target_link_libraries(basicAttributeTest SlctkAttribute)


add_test(basicAttributeTest ${EXECUTABLE_OUTPUT_PATH}/basicAttributeTest)

smtk_public_headers(helpers.h)

set(commonTests
  unitUUID
)

foreach (test ${commonTests})
  add_executable(${test}  ${test}.cxx)
  target_link_libraries(${test} SMTKCore)
  add_test(${test} ${EXECUTABLE_OUTPUT_PATH}/${test})
endforeach()

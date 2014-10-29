smtk_public_headers(helpers.h)

set(commonTests
  unitUUID
  unitPaths
)

foreach (test ${commonTests})
  add_executable(${test} MACOSX_BUNDLE  ${test}.cxx)
  target_link_libraries(${test} SMTKCore)
  add_test(
    NAME ${test}
    COMMAND $<TARGET_FILE:${test}>)
endforeach()

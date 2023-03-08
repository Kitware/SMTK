set(smtkProjectPythonTests
  testBasicProject
  testProject
)

# Additional tests that require SMTK_DATA_DIR
set(smtkProjectPythonDataTests
)

foreach (test ${smtkProjectPythonTests})
  smtk_add_test_python(${test}Py ${test}.py --src-dir=${smtk_SOURCE_DIR})
endforeach()

if (SMTK_DATA_DIR)
  foreach (test ${smtkProjectPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py --data-dir=${SMTK_DATA_DIR})
  endforeach()
endif()

add_executable(testPythonProject TestPythonProject.cxx)
target_compile_definitions(testPythonProject PRIVATE "SMTK_DATA_DIR=\"${SMTK_DATA_DIR}\"")
target_link_libraries(testPythonProject smtkCore)
add_test(NAME testPythonProject
  COMMAND $<TARGET_FILE:testPythonProject>
  "${CMAKE_CURRENT_SOURCE_DIR}/testProject.py")
set_tests_properties(testPythonProject PROPERTIES
  LABELS "Project"
  ENVIRONMENT PYTHONDONTWRITEBYTECODE=1)

add_executable(TestManagersAccess TestManagersAccess.cxx)
target_link_libraries(TestManagersAccess smtkCore)
add_test(NAME TestManagersAccess
  COMMAND $<TARGET_FILE:TestManagersAccess>
  "${CMAKE_CURRENT_SOURCE_DIR}/test_managers_operation.py")
set_tests_properties(TestManagersAccess PROPERTIES
  LABELS "Project"
  ENVIRONMENT PYTHONDONTWRITEBYTECODE=1)

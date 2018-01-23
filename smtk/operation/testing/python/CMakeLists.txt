# tests that require SMTK_DATA_DIR
set(smtkOperationPythonDataTests
)

#only run these tests if we have a valid data directory and we have a moab
#built with hdf5
if (SMTK_DATA_DIR)
  foreach (test ${smtkOperationPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR} )
    set_tests_properties( ${test}Py PROPERTIES LABELS "Operation" )
  endforeach()
endif()

add_executable(testImportPythonOperator TestImportPythonOperator.cxx)
target_compile_definitions(testImportPythonOperator PRIVATE "SMTK_DATA_DIR=\"${SMTK_DATA_DIR}\"")
target_link_libraries(testImportPythonOperator smtkCore)
add_test(NAME testImportPythonOperator
  COMMAND $<TARGET_FILE:testImportPythonOperator>
  "${CMAKE_CURRENT_SOURCE_DIR}/test_op.py")
set_tests_properties(testImportPythonOperator PROPERTIES
  LABELS "Operation"
  ENVIRONMENT PYTHONDONTWRITEBYTECODE=1)

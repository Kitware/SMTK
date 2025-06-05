set(smtkOperationPythonTests
  testConstructedAPI
)

# tests that require SMTK_DATA_DIR
set(smtkOperationPythonDataTests
)

if (SMTK_ENABLE_POLYGON_SESSION)
  list(APPEND smtkOperationPythonDataTests
    testOperationHandler
  )
endif()

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  list(APPEND smtkOperationPythonDataTests
    testOperationTracing
  )
endif()

foreach (test ${smtkOperationPythonTests})
  smtk_add_test_python(${test}Py ${test}.py --temp-dir=${CMAKE_BINARY_DIR}/Testing/Temporary)
endforeach()

#only run these tests if we have a valid data directory and we have vtk session
if (SMTK_DATA_DIR AND SMTK_ENABLE_VTK_SESSION)
  foreach (test ${smtkOperationPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR} )
    set_tests_properties( ${test}Py PROPERTIES LABELS "Operation" )
  endforeach()
endif()

add_executable(testImportPythonOperation TestImportPythonOperation.cxx)
target_compile_definitions(testImportPythonOperation PRIVATE "SMTK_DATA_DIR=\"${SMTK_DATA_DIR}\"")
target_link_libraries(testImportPythonOperation smtkCore)
add_test(NAME testImportPythonOperation
  COMMAND $<TARGET_FILE:testImportPythonOperation>
  "${CMAKE_CURRENT_SOURCE_DIR}/test_op.py")
set_tests_properties(testImportPythonOperation PROPERTIES
  LABELS "Operation"
  ENVIRONMENT PYTHONDONTWRITEBYTECODE=1)

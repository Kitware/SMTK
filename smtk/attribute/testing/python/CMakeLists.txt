find_package( PythonInterp 2.7 )

# Additional tests that require SMTK_DATA_DIR
set(smtkModelPythonDataTests
  copyDefinitionTest
  copyAttributeTest
)


if( PYTHONINTERP_FOUND )
  set(LIB_ENV_VAR "")
  if(APPLE AND UNIX)
    set(LIB_ENV_VAR "DYLD_LIBRARY_PATH=${SHIBOKEN_LIBRARY_PATH}")
  elseif(UNIX)
    set(LIB_ENV_VAR "LD_LIBRARY_PATH=${SHIBOKEN_LIBRARY_PATH}")
  endif()
  set(SIBOKEN_SMTK_PYTHON "")
  if(UNIX)
    set(SHIBOKEN_SMTK_PYTHON "${CMAKE_BINARY_DIR}:${SHIBOKEN_LIBRARY_PATH}")
  else() #windows
    set(SHIBOKEN_SMTK_PYTHON "${CMAKE_BINARY_DIR};${SHIBOKEN_LIBRARY_PATH}")
  endif()
  #message("${LIB_ENV_VAR}   PYTHONPATH=${SIBOKEN_SMTK_PYTHON}")
  #message("${SHIBOKEN_LIBRARY_PATH}")
   add_test(associationTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/associationTest.py)
   add_test(basicAttributeTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/basicAttributeTest.py)
   add_test(basicAttributeDefinitionTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/basicAttributeDefinitionTest.py)
   add_test(basicAttributeDerivationTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/basicAttributeDerivationTest.py)
   add_test(basicAttributeXMLWriterTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/basicAttributeXMLWriterTest.py dummypy.sbi)
   add_test(expressintTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/expressionTest.py)
   add_test(expressionTest2Py ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/expressionTest2.py)
   add_test(attributeAutoNamingTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/attributeAutoNamingTest.py)
   add_test(attributeReferencingTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/attributeReferencingTest.py)
   add_test(categoryTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/categoryTest.py)
   add_test(attributeFindItemTestPy ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/attributeFindItemTest.py)
  set_tests_properties( associationTestPy basicAttributeTestPy basicAttributeDefinitionTestPy
                        basicAttributeDerivationTestPy basicAttributeXMLWriterTestPy
                        expressintTestPy expressionTest2Py
                        attributeAutoNamingTestPy attributeAutoNamingTestPy
                        attributeReferencingTestPy categoryTestPy attributeFindItemTestPy
                        PROPERTIES ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}" )

  if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/ReadMe.mkd)
    foreach (test ${smtkModelPythonDataTests})
      add_test(${test}Py
               ${PYTHON_EXECUTABLE}
               ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
               ${SMTK_DATA_DIR})
      set_tests_properties(${test}Py
        PROPERTIES
          ENVIRONMENT "PYTHONPATH=${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
      )
    endforeach()
  endif()

else()
  message("could not find python interp")
endif()

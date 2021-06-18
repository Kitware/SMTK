## Tests that require SMTK_DATA_DIR
# set(smtkOscillatorPythonDataTests)
#
## Only run these tests if we have a valid data directory
# if (SMTK_DATA_DIR)
#   foreach (test ${smtkOscillatorPythonDataTests})
#     smtk_add_test_python(${test}Py ${test}.py
#       --data-dir=${SMTK_DATA_DIR} )
#     set_tests_properties( ${test}Py PROPERTIES LABELS "OscillatorSession" )
#   endforeach()
# endif()

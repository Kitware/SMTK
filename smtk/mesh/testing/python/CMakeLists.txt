

# tests that require SMTK_DATA_DIR
set(smtkMeshPythonDataTests
  extractTessellation
  iterateMesh
  simple
)

#only run these tests if we have a valid data directory and we have a moab
#built with hdf5
if (SMTK_DATA_DIR
    AND EXISTS ${SMTK_DATA_DIR}/cmb-testing-data.marker
    AND using_hdf)
  foreach (test ${smtkMeshPythonDataTests})
    add_test(${test}Py
             ${PYTHON_EXECUTABLE}
             ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
             --data-dir=${SMTK_DATA_DIR} )
    set_tests_properties(${test}Py
      PROPERTIES
        ENVIRONMENT "PYTHONPATH=${VTKPY_DIR}${VTKPY_DIR}${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
    )
    set_tests_properties( ${test}Py PROPERTIES LABELS "Mesh" )
  endforeach()
endif()

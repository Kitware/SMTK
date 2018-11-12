

# tests that require SMTK_DATA_DIR
set(smtkMeshPythonDataTests
  importAndWriteGenesisFile
  loadExodusFile
)

if (SMTK_ENABLE_VTK_SUPPORT)
  list (APPEND smtkMeshPythonDataTests eulerRatio)
endif()

#only run these tests if we have a valid data directory and we have a moab
#built with hdf5
if (SMTK_DATA_DIR)
  foreach (test ${smtkMeshPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR}
      --temp-dir=${CMAKE_BINARY_DIR}/Testing/Temporary)
    set_tests_properties( ${test}Py PROPERTIES LABELS "MeshSession" )
  endforeach()
endif()

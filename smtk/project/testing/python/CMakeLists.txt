# Project Manager test using Mesh Session
if (SMTK_ENABLE_MESH_SESSION AND SMTK_DATA_DIR)
  set(mstest "projectManagerMeshSessionTest")
  smtk_add_test_python(${mstest}Py ${mstest}.py
    --data-dir=${SMTK_DATA_DIR}
    --temp-dir=${CMAKE_BINARY_DIR}/Testing/Temporary)
endif()

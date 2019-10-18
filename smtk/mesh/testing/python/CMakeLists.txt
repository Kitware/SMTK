

# tests that require SMTK_DATA_DIR
set(smtkMeshPythonDataTests
  extractTessellation
  cellField
  meshMetrics
  pointField
  simple
  iterateMesh
)

if(SMTK_ENABLE_MESH_SESSION AND SMTK_ENABLE_VTK_SUPPORT)
list(APPEND smtkMeshPythonDataTests
  elevateMeshOnStructuredGrid
  snapPointsToSurface
  )
endif()

#only run these tests if we have a valid data directory and we have a moab
#built with hdf5
if (SMTK_DATA_DIR)
  foreach (test ${smtkMeshPythonDataTests})
    smtk_add_test_python(${test}Py ${test}.py
      --data-dir=${SMTK_DATA_DIR} )
    set_tests_properties( ${test}Py PROPERTIES LABELS "Mesh" )
  endforeach()
endif()

#if (SMTK_ENABLE_POLYGON_SESSION)
#  add_executable(testRuntimePyOperation TestRuntimePyOperation.cxx)
#  target_compile_definitions(testRuntimePyOperation PRIVATE "SMTK_DATA_DIR=\"${SMTK_DATA_DIR}\"")
#  target_link_libraries(testRuntimePyOperation smtkCore smtkDelaunayExt smtkPolygonSession ${Boost_LIBRARIES})
#    add_test(NAME testMyElevateMesh
#      COMMAND $<TARGET_FILE:testRuntimePyOperation>
#      "${CMAKE_CURRENT_SOURCE_DIR}/my_elevate_mesh.py")
#    set_tests_properties(testMyElevateMesh PROPERTIES LABELS "Mesh" ENVIRONMENT PYTHONDONTWRITEBYTECODE=1)
#endif()

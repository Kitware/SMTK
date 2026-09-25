# This test loads ParaView plugins to register its Qt job queues.
if (NOT SMTK_ENABLE_PARAVIEW_SUPPORT)
  return()
endif()

# Tests that don't require any command-line arguments
set(plainTests)
# The queue test loads SMTK plugins through ParaView during setup.
if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  list(APPEND plainTests queueJob)
endif()

foreach(test ${plainTests})
  smtk_add_test_python(${test}Py ${test}.py)
  set_tests_properties(${test}Py PROPERTIES SKIP_RETURN_CODE 125 LABELS "Job")
endforeach()

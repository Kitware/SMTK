# This test loads ParaView plugins to register its Qt job queues.
if (NOT SMTK_ENABLE_PARAVIEW_SUPPORT OR NOT SMTK_ENABLE_QT_SUPPORT)
  return()
endif()

# The queues need a Qt application even when driven from Python.
add_executable(queueJobPythonDriver queueJobPythonDriver.cxx)
target_link_libraries(queueJobPythonDriver PRIVATE Qt5::Core Python3::Python)
add_test(NAME queueJobPy
  COMMAND queueJobPythonDriver "${CMAKE_CURRENT_SOURCE_DIR}/queueJob.py")
# Bound runtime setup and image pulls as well as the Python job-completion waits.
# unittest handles per-case skips; a missing Podman must not skip the shell cases.
set_tests_properties(queueJobPy PROPERTIES
  ENVIRONMENT "${smtk_python_test_environment}"
  LABELS "Job"
  TIMEOUT 180)
smtk_test_apply_sanitizer(queueJobPy)

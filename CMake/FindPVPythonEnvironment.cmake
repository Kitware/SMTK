# Find paraview
find_package(ParaView)

if (NOT TARGET pvpython)
  message(FATAL_ERROR "Could not locate target pvpython. Either build ParaView with python support or disable SMTK's python support.")
endif()

# Access the location of ParaView's pvpython
get_target_property(PVPYTHON_EXE pvpython LOCATION)

# Execute a python script that prints the correct PYTHONPATH for ParaView.
#
# NOTE: pvpython currently outputs to standard error. As a workaround, we
# capture both standard out and standard error for PYTHONPATH (at least one of
# them should be right!).
execute_process(
  COMMAND ${PVPYTHON_EXE}
  ${PROJECT_SOURCE_DIR}/CMake/FindPVPythonEnvironment.py
  RESULT_VARIABLE rv
  OUTPUT_VARIABLE PARAVIEW_PYTHONPATH
  ERROR_VARIABLE PARAVIEW_PYTHONPATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE
)

if (NOT rv EQUAL 0)
  message(FATAL_ERROR "Could not determine ParaView's PYTHONPATH; return value was ${rv} and output was ${PARAVIEW_PYTHONPATH}.")
endif()

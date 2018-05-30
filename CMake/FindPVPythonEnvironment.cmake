# Find paraview
find_package(ParaView)

# On some operating systems and in certain configurations, ParaView creates a
# launcher for each of its executables. The launcher is responsible for setting
# up the appropriate environment and then launching the real executable. When a
# launcher is not available, it is likely because the rpath for the real
# executable has been set to accomplish the task of configuring the environment.
# We first check if ParaView has a launcher for pvpython, and fall back to using
# the real pvpython.
set(PVPYTHON_TARGET pvpython-launcher)
if (NOT TARGET ${PVPYTHON_TARGET})
  set(PVPYTHON_TARGET pvpython)
endif()

if (NOT TARGET ${PVPYTHON_TARGET})
  message(FATAL_ERROR "Could not locate target pvpython. Either build ParaView with python support or disable SMTK's python support.")
endif()

# Access the location of ParaView's pvpython
get_target_property(PVPYTHON_EXE ${PVPYTHON_TARGET} LOCATION)

# Execute a python script that prints the correct PYTHONPATH for ParaView.
#
# NOTE: pvpython currently outputs to standard error. As a workaround, we
# capture both standard out and standard error for PYTHONPATH (at least one of
# them should be right!).
execute_process(
  COMMAND ${PVPYTHON_EXE}
  ${PROJECT_SOURCE_DIR}/CMake/FindPVPythonEnvironment.py
  RESULT_VARIABLE rv
  OUTPUT_VARIABLE out
  ERROR_VARIABLE out
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE
)

if (NOT rv EQUAL 0)
  message(FATAL_ERROR "Could not determine ParaView's PYTHONPATH; return value was ${rv} and output was ${out}.")
endif()

# ParaView may have a verbose input. We search from the end of the output stream
# for the keyword "PARAVIEW_PYTHONPATH=" and take the rest of the output as
# ParaView's python path.
set(marker "PARAVIEW_PYTHONPATH=")
string(LENGTH ${marker} markerlen)
string(FIND ${out} ${marker} start REVERSE)
math(EXPR start "${start} + ${markerlen}")
string(LENGTH ${out} len)
math(EXPR len "${len} - ${start}")
string(SUBSTRING ${out} ${start} ${len} PARAVIEW_PYTHONPATH)

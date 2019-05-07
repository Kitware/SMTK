set(PVPYTHON_EXE ${ParaView_CLEXECUTABLES_DIR}/pvpython${CMAKE_EXECUTABLE_SUFFIX})

if (NOT EXISTS ${PVPYTHON_EXE})

  # Find paraview
  find_package(ParaView)

  if (NOT TARGET ParaView::pvpython)
    message(FATAL_ERROR "Could not locate target pvpython. Either build ParaView with python support or disable SMTK's python support.")
  endif()

  # Access the location of ParaView's pvpython
  get_target_property(PVPYTHON_EXE ParaView::pvpython LOCATION)
endif()

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
# for the keyword "PARAVIEW_PYTHONPATH=" and take the rest of the output up to a
# newline character as ParaView's python path.
set(marker "PARAVIEW_PYTHONPATH=")
string(LENGTH ${marker} markerlen)
string(FIND ${out} ${marker} start REVERSE)
math(EXPR start "${start} + ${markerlen}")
string(LENGTH ${out} len)
math(EXPR len "${len} - ${start}")
string(SUBSTRING ${out} ${start} ${len} PARAVIEW_PYTHONPATH)
string(FIND ${PARAVIEW_PYTHONPATH} "\n" end)
string(SUBSTRING ${PARAVIEW_PYTHONPATH} 0 ${end} PARAVIEW_PYTHONPATH)

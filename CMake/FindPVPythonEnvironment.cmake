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
  OUTPUT_VARIABLE out
  ERROR_VARIABLE out
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE
)

if (NOT rv EQUAL 0)
  # Sometimes pvpython's LOCATION property points to the build location of
  # pvpython, and sometimes this pvpython does not successfully run. If this
  # happens, as a workaround we look for pvpython in our own runtime directory.
  get_filename_component(PVPYTHON_NAME ${PVPYTHON_EXE} NAME)
  set(PVPYTHON_EXE "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/${PVPYTHON_NAME}")

  if (EXISTS ${PVPYTHON_EXE})
    # If we can find the installed version of pvpython, we try our
    # FindPVPythonEnvironment.py one more time.
    execute_process(
      COMMAND ${PVPYTHON_EXE}
      ${PROJECT_SOURCE_DIR}/CMake/FindPVPythonEnvironment.py
      RESULT_VARIABLE rv
      OUTPUT_VARIABLE out
      ERROR_VARIABLE out
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE
      )
  endif ()
endif ()

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

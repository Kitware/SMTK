set(matplotlibPySrcs
  __init__.py
  render_mesh.py
  )

set(matplotlib_pymodulefiles)

smtk_pyoperator_xml("${CMAKE_CURRENT_SOURCE_DIR}/render_mesh.sbt" matplotlib_pymodulefiles)

foreach(pyfile ${matplotlibPySrcs})
  get_filename_component(filename ${pyfile} NAME)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/${pyfile}"
    "${CMAKE_CURRENT_BINARY_DIR}/${filename}" @ONLY
    )
  list(APPEND matplotlib_pymodulefiles "${CMAKE_CURRENT_BINARY_DIR}/${filename}")
endforeach()
install(
  FILES ${matplotlib_pymodulefiles}
  DESTINATION "${SMTK_PYTHON_MODULEDIR}/smtk/matplotlib"
  )

add_subdirectory(plugin)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

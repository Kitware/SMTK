set(source
  RegisterOperations.cxx
  )
set(headers
  RegisterOperations.h
  )
set(matplotlibPySrcs
  __init__.py
  render_mesh.py
  )

set(matplotlib_pymodulefiles)

set(matplotlib_pyxmlfiles)
# TODO consider updating to smtk_encode_file() if matplotlib is re-enabled.
smtk_operation_xml(
  "${CMAKE_CURRENT_SOURCE_DIR}/render_mesh.sbt" matplotlib_pyxmlfiles
  PYTHON
  )
message("XXX Matplotlib ${matplotlib_pyxmlfiles}")

foreach(pyfile ${matplotlibPySrcs})
  get_filename_component(filename ${pyfile} NAME)
  file(RELATIVE_PATH relativefile ${PROJECT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/${pyfile}")
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/${pyfile}"
    "${CMAKE_BINARY_DIR}/${relativefile}" @ONLY
    )
  list(APPEND matplotlib_pymodulefiles "${CMAKE_BINARY_DIR}/${relativefile}")
endforeach()

foreach(pyfile ${matplotlib_pyxmlfiles})
  get_filename_component(filename ${pyfile} NAME)
  file(RELATIVE_PATH relativefile ${PROJECT_BINARY_DIR} ${pyfile})
  configure_file(
    "${pyfile}"
    "${CMAKE_BINARY_DIR}/${relativefile}" @ONLY
    )
  list(APPEND matplotlib_pymodulefiles "${CMAKE_BINARY_DIR}/${relativefile}")
endforeach()
message("XXX Matplotlib ${matplotlib_pymodulefiles}")

file(RELATIVE_PATH relativedir ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
install(
  FILES ${matplotlib_pymodulefiles}
  DESTINATION "${SMTK_PYTHON_MODULEDIR}/${relativedir}"
  )

add_library(smtkMatplotlibExt ${source})
target_link_libraries(smtkMatplotlibExt
  PRIVATE
    smtkCore
)

smtk_export_header(smtkMatplotlibExt Exports.h)
smtk_public_headers(smtkMatplotlibExt ${headers})
smtk_install_library(smtkMatplotlibExt)

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

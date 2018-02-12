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

smtk_pyoperation_xml("${CMAKE_CURRENT_SOURCE_DIR}/render_mesh.sbt" matplotlib_pymodulefiles)

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
  DESTINATION "${SMTK_PYTHON_MODULEDIR}/smtk/extension/matplotlib"
  )

add_library(smtkMatplotlibExt ${source})
target_link_libraries(smtkMatplotlibExt
  PRIVATE
    smtkCore
)

smtk_export_header(smtkMatplotlibExt Exports.h)
smtk_public_headers(${headers})
smtk_install_library(smtkMatplotlibExt)

add_subdirectory(environment)

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_subdirectory(plugin)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

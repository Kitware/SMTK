pybind11_add_module(smtkPybindIOVTK PybindIOVTK.cxx)
target_include_directories(smtkPybindIOVTK PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  )
target_link_libraries(smtkPybindIOVTK LINK_PUBLIC smtkCore smtkIOVTK)
set_target_properties(smtkPybindIOVTK
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    COMPILE_FLAGS ${SMTK_PYBIND11_FLAGS}
)
install(TARGETS smtkPybindIOVTK DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py"
  "${PROJECT_BINARY_DIR}/smtk/io/vtk/__init__.py" @ONLY
  )

install(CODE
  "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
       configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
       ${CMAKE_INSTALL_PREFIX}/${SMTK_PYTHON_MODULEDIR}/smtk/io/vtk/__init__.py )"
  )

pybind11_add_module(smtkPybindIO PybindIO.cxx)
target_include_directories(smtkPybindIO PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  )
target_link_libraries(smtkPybindIO LINK_PUBLIC smtkCore)
set_target_properties(smtkPybindIO
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    COMPILE_FLAGS ${SMTK_PYBIND11_FLAGS}
)
install(TARGETS smtkPybindIO DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py"
  "${PROJECT_BINARY_DIR}/smtk/io/__init__.py" @ONLY
  )

install(CODE
  "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
       configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
       ${CMAKE_INSTALL_PREFIX}/${SMTK_PYTHON_MODULEDIR}/smtk/io/__init__.py )"
  )

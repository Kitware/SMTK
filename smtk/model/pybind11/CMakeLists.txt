pybind11_add_module(smtkPybindModel PybindModel.cxx)
target_include_directories(smtkPybindModel PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  )
target_link_libraries(smtkPybindModel LINK_PUBLIC smtkCore)
set_target_properties(smtkPybindModel
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    COMPILE_FLAGS "-Wno-shadow"
)
install(TARGETS smtkPybindModel DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py"
  "${PROJECT_BINARY_DIR}/smtk/model/__init__.py" @ONLY
  )

install(CODE
  "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
       configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
       ${CMAKE_INSTALL_PREFIX}/${SMTK_PYTHON_MODULEDIR}/smtk/model/__init__.py )"
  )

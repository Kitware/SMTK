pybind11_add_module(smtkPybindAttribute PybindAttribute.cxx)
target_include_directories(smtkPybindAttribute PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  )
target_link_libraries(smtkPybindAttribute LINK_PUBLIC smtkCore)
set_target_properties(smtkPybindAttribute
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    COMPILE_FLAGS "-Wno-shadow"
)
install(TARGETS smtkPybindAttribute DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py"
  "${PROJECT_BINARY_DIR}/smtk/attribute/__init__.py" @ONLY
  )

install(CODE
  "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
       configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
       ${CMAKE_INSTALL_PREFIX}/${SMTK_PYTHON_MODULEDIR}/smtk/attribute/__init__.py )"
  )

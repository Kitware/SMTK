pybind11_add_module(smtkPybindCGMSession PybindCGMSession.cxx)
target_include_directories(smtkPybindCGMSession PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  )
target_link_libraries(smtkPybindCGMSession LINK_PUBLIC smtkCore smtkCGMSession LINK_PRIVATE ${CGM_LIBRARIES})
set_target_properties(smtkPybindCGMSession
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    COMPILE_FLAGS "-Wno-shadow"
)
install(TARGETS smtkPybindCGMSession DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
  "${PROJECT_BINARY_DIR}/smtk/bridge/cgm/__init__.py" @ONLY
  )

install(CODE
  "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
       configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in
       ${CMAKE_INSTALL_PREFIX}/${SMTK_PYTHON_MODULEDIR}/smtk/bridge/cgm/__init__.py )"
  )

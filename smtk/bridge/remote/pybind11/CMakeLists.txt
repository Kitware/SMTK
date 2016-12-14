pybind11_add_module(smtkPybindRemoteSession PybindRemoteSession.cxx)
target_include_directories(smtkPybindRemoteSession PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  )
target_link_libraries(smtkPybindRemoteSession LINK_PUBLIC
  smtkCore
  smtkRemoteSession
  RemusCommon
  RemusProto
  RemusClient
  RemusWorker
)
set_target_properties(smtkPybindRemoteSession
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
)
install(TARGETS smtkPybindRemoteSession DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
  "${PROJECT_BINARY_DIR}/smtk/bridge/remote/__init__.py" @ONLY
  )

install(CODE
  "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
       configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in
       ${CMAKE_INSTALL_PREFIX}/${SMTK_PYTHON_MODULEDIR}/smtk/bridge/remote/__init__.py )"
  )

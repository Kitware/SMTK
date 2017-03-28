pybind11_add_module(smtkPybindMultiscaleSession PybindMultiscaleSession.cxx)
target_include_directories(smtkPybindMultiscaleSession PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  )
set(import_vtk_depends
    vtkCommonDataModel
    vtkPythonInterpreter
    vtkIOXdmf2
    vtkImagingCore
    vtkFiltersGeneral
    vtkFiltersGeometry
    )
target_link_libraries(smtkPybindMultiscaleSession LINK_PUBLIC smtkCore smtkMultiscaleSession LINK_PRIVATE ${import_vtk_depends})
set_target_properties(smtkPybindMultiscaleSession
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
)
install(TARGETS smtkPybindMultiscaleSession DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
  "${PROJECT_BINARY_DIR}/smtk/bridge/multiscale/__init__.py" @ONLY
  )

install(CODE
  "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
       configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in
       ${CMAKE_INSTALL_PREFIX}/${SMTK_PYTHON_MODULEDIR}/smtk/bridge/multiscale/__init__.py )"
  )

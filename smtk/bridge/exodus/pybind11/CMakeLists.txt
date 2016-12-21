pybind11_add_module(smtkPybindExodusSession PybindExodusSession.cxx)
target_include_directories(smtkPybindExodusSession PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  )
set(import_vtk_depends
    vtkIOExodus
    vtkIONetCDF
    vtkIOXML
    vtkImagingCore
    vtkFiltersGeneral
    vtkIOLegacy
    vtkIOParallel
    vtkIOParallelExodus
    vtkIOGeometry
    vtkFiltersGeometry
    )
target_link_libraries(smtkPybindExodusSession LINK_PUBLIC smtkCore smtkExodusSession vtkCommonDataModel LINK_PRIVATE ${iport_vtk_depends})
set_target_properties(smtkPybindExodusSession
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    COMPILE_FLAGS "-Wno-shadow"
)
install(TARGETS smtkPybindExodusSession DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
  "${PROJECT_BINARY_DIR}/smtk/bridge/exodus/__init__.py" @ONLY
  )

install(CODE
  "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
       configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in
       ${CMAKE_INSTALL_PREFIX}/${SMTK_PYTHON_MODULEDIR}/smtk/bridge/exodus/__init__.py )"
  )

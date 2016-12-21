pybind11_add_module(smtkPybindDiscreteSession PybindDiscreteSession.cxx)
target_include_directories(smtkPybindDiscreteSession PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  )
set(import_vtk_depends
    vtkCommonExecutionModel
    vtkFiltersCore
    vtkFiltersGeometry
    vtkFiltersSources
    vtkGeovisCore
    vtkIOGDAL
    vtkIOParallel
    vtkRenderingCore
    vtkRenderingFreeType
    vtkRenderingGL2PSOpenGL2
    vtkRenderingContextOpenGL2
    vtkRenderingOpenGL2
    vtksys
    )
target_link_libraries(smtkPybindDiscreteSession LINK_PUBLIC smtkCore smtkDiscreteSession vtkCommonDataModel LINK_PRIVATE ${import_vtk_depends})
set_target_properties(smtkPybindDiscreteSession
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    COMPILE_FLAGS "-Wno-shadow"
)
install(TARGETS smtkPybindDiscreteSession DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
  "${PROJECT_BINARY_DIR}/smtk/bridge/discrete/__init__.py" @ONLY
  )

install(CODE
  "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
       configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in
       ${CMAKE_INSTALL_PREFIX}/${SMTK_PYTHON_MODULEDIR}/smtk/bridge/discrete/__init__.py )"
  )

pybind11_add_module(smtkPybindPolygonSession PybindPolygonSession.cxx)
target_include_directories(smtkPybindPolygonSession PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  )
set(import_vtk_depends)
if(SMTK_ENABLE_VTK_SUPPORT)
  list(APPEND import_vtk_depends
    vtkCommonCore
    vtkCommonDataModel
    vtkFiltersGeometry
    vtkInteractionStyle
    vtkIOParallel
    vtkRenderingCore
    vtkRenderingFreeType
    vtkRenderingOpenGL2
    vtkSMTKReaderExt
    vtksys
    )
endif()
target_link_libraries(smtkPybindPolygonSession LINK_PUBLIC smtkCore smtkPolygonSession LINK_PRIVATE ${import_vtk_depends} ${Boost_LIBRARIES})
set_target_properties(smtkPybindPolygonSession
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    COMPILE_FLAGS ${SMTK_PYBIND11_FLAGS}
)
install(TARGETS smtkPybindPolygonSession DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
  "${PROJECT_BINARY_DIR}/smtk/bridge/polygon/__init__.py" @ONLY
  )

install(CODE
  "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
       configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in
       ${CMAKE_INSTALL_PREFIX}/${SMTK_PYTHON_MODULEDIR}/smtk/bridge/polygon/__init__.py )"
  )

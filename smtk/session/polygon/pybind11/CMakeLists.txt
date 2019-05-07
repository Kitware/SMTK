set(library_name "_smtkPybindPolygonSession")
set(module_path "session/polygon")
set(build_path "${CMAKE_BINARY_DIR}/smtk/${module_path}")
set(install_path "${SMTK_PYTHON_MODULEDIR}/smtk/${module_path}")

pybind11_add_module(${library_name} PybindPolygonSession.cxx)
target_include_directories(${library_name} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  )
set(import_vtk_depends)
if(SMTK_ENABLE_VTK_SUPPORT)
  list(APPEND import_vtk_depends
    vtkSMTKReaderExt
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::FiltersGeometry
    VTK::InteractionStyle
    VTK::IOParallel
    VTK::RenderingCore
    VTK::RenderingFreeType
    VTK::RenderingOpenGL2
    VTK::vtksys
    )
endif()
target_link_libraries(${library_name} LINK_PUBLIC smtkCore smtkPolygonSession LINK_PRIVATE ${import_vtk_depends} ${Boost_LIBRARIES})
vtk_module_autoinit(
  TARGETS ${library_name}
  MODULES ${import_vtk_depends})
if(SMTK_ENABLE_VTK_SUPPORT)
  target_compile_definitions(${library_name} PUBLIC VTK_SUPPORT)
endif()
set_target_properties(${library_name}
  PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    COMPILE_FLAGS ${SMTK_PYBIND11_FLAGS}
    LIBRARY_OUTPUT_DIRECTORY "${build_path}"
  )

# Install library
install(TARGETS ${library_name} DESTINATION "${install_path}")

# Create and install module __init__.py
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
  "${build_path}/__init__.py" @ONLY
  )

install(
  FILES "${build_path}/__init__.py"
  DESTINATION "${install_path}"
  )

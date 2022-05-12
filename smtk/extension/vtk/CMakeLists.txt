set(module_files
  "${CMAKE_CURRENT_SOURCE_DIR}/filter/vtk.module"
  "${CMAKE_CURRENT_SOURCE_DIR}/geometry/vtk.module"
  "${CMAKE_CURRENT_SOURCE_DIR}/mesh/vtk.module"
  "${CMAKE_CURRENT_SOURCE_DIR}/model/vtk.module"
  "${CMAKE_CURRENT_SOURCE_DIR}/operators/vtk.module"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/vtk.module"
  "${CMAKE_CURRENT_SOURCE_DIR}/widgets/vtk.module"
)
if (SMTK_MARKUP_SUPPORT_ENABLED)
  list(APPEND module_files
    "${CMAKE_CURRENT_SOURCE_DIR}/markup/vtk.module"
  )
endif()
vtk_module_scan(
  MODULE_FILES ${module_files}
  PROVIDES_MODULES vtk_modules
  HIDE_MODULES_FROM_CACHE ON
  WANT_BY_DEFAULT ON)
vtk_module_build(
  MODULES ${vtk_modules}
  PACKAGE SMTKVTKModules
  INSTALL_EXPORT SMTKVTKModules
  CMAKE_DESTINATION ${SMTK_INSTALL_CONFIG_DIR}
  HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${SMTK_VERSION}"
  TEST_DIRECTORY_NAME "NONE")

set_property(GLOBAL
  PROPERTY
    _smtk_vtk_extention_modules "${vtk_modules}")

if (SMTK_ENABLE_PYTHON_WRAPPING)
  set(vtk_python_wrappable_modules
    vtkSMTKGeometryExt
    vtkSMTKOperationsExt
    vtkSMTKSourceExt
    vtkSMTKWidgetsExt
    # vtkSMTKMarkupExt doesn't have anything in need of wrapping yet.
  )
  set(vtk_modules_to_wrap)
  foreach (vtk_python_wrappable_module IN LISTS vtk_python_wrappable_modules)
    if (TARGET "${vtk_python_wrappable_module}")
      list(APPEND vtk_modules_to_wrap
        "${vtk_python_wrappable_module}")
    endif ()
  endforeach ()
  vtk_module_wrap_python(
    MODULES         ${vtk_modules_to_wrap}
    INSTALL_EXPORT  smtk
    INSTALL_HEADERS OFF
    PYTHON_PACKAGE  "smtk.extension.vtk._modules"
    #MODULE_DESTINATION  "${PARAVIEW_PYTHON_SITE_PACKAGES_SUFFIX}"
    #CMAKE_DESTINATION   "${paraview_cmake_destination}/vtk"
    WRAPPED_MODULES vtk_python_wrapped_modules
    TARGET          vtksmtkpythonmodules)
  add_subdirectory(pybind11)
endif()

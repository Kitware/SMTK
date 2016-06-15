set (__dependencies)

vtk_module(vtkSMTKFilterExt
  DEPENDS
    vtkCommonCore
  PRIVATE_DEPENDS
    vtkFiltersGeneral
    ${__dependencies}
  TEST_DEPENDS
  EXCLUDE_FROM_WRAP_HIERARCHY
)

set (__dependencies)
list(APPEND __dependencies vtksys)

vtk_module(vtkSMTKFilterExt
  DEPENDS
    vtkCommonCore
  PRIVATE_DEPENDS
    vtkFiltersGeneral
    ${__dependencies}
  TEST_DEPENDS
  EXCLUDE_FROM_WRAP_HIERARCHY
)

set (__dependencies)

vtk_module(vtkSMTKOperatorsExt
  DEPENDS
    vtkCommonCore
  PRIVATE_DEPENDS
    vtkCommonDataModel
    vtkCommonExecutionModel
    vtkFiltersGeneral
    vtkIOGDAL
    vtkIOXML
    vtkSMTKFilterExt
    ${__dependencies}
  TEST_DEPENDS
  EXCLUDE_FROM_WRAP_HIERARCHY
)

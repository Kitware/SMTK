set (__dependencies)

vtk_module(vtkPolygonOperatorsExt
  DEPENDS
    vtkCommonDataModel
  PRIVATE_DEPENDS
    ${__dependencies}
  TEST_DEPENDS
  EXCLUDE_FROM_WRAP_HIERARCHY
)

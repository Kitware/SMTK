set (__dependencies)
list (APPEND __dependencies
      vtkCommonDataModel
      vtkCommonSystem
      vtkCommonTransforms
      vtkIOXML
      vtksys
      vtkFiltersCore
      vtkFiltersGeneral
      )

if (SMTK_ENABLE_VXL_SUPPORT)
  vtk_module(vtkSMTKVXLExt
    DEPENDS
      vtkCommonCore
      vtkCommonExecutionModel
    PRIVATE_DEPENDS
      ${__dependencies}
    TEST_DEPENDS
  EXCLUDE_FROM_PYTHON_WRAPPING
  )
endif()

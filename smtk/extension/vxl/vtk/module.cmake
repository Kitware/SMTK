set (__dependencies)
list (APPEND __dependencies vtksys)

if (SMTK_ENABLE_VXL_SUPPORT)
  vtk_module(vtkSMTKVXLExt
    DEPENDS
      vtkCommonCore
    PRIVATE_DEPENDS
      ${__dependencies}
    TEST_DEPENDS
    EXCLUDE_FROM_WRAP_HIERARCHY
  )
endif()

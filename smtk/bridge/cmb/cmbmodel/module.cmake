set (__dependencies)

if (PARAVIEW_ENABLE_PYTHON)
  list(APPEND __dependencies
      vtkPythonInterpreter)
endif (PARAVIEW_ENABLE_PYTHON)

vtk_module(vtkSMTKCMBModel
  GROUPS
   CmbCore
  DEPENDS
   vtkSMTKDiscreteModel
   vtkPVServerManagerCore
   vtkPVClientServerCoreRendering
   vtkPVVTKExtensionsRendering
   vtkFiltersCore
   vtkRenderingCore
   vtkGeovisCore
   vtksys
   ${__dependencies}
 EXCLUDE_FROM_WRAP_HIERARCHY
)

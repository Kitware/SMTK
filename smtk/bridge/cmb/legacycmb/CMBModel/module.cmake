set (__dependencies)

if (PARAVIEW_ENABLE_PYTHON)
  list(APPEND __dependencies
      vtkPythonInterpreter)
endif (PARAVIEW_ENABLE_PYTHON)

vtk_module(vtkCmbDiscreteModel
  GROUPS
   CmbCore
  DEPENDS
   vtkDiscreteModel
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

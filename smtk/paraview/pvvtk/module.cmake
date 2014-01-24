vtk_module(pvSMTK
  DEPENDS
    vtkClientServer
    vtkCommonCore
    vtkCommonDataModel
    vtkCommonExecutionModel
    vtkPVClientServerCoreRendering
#  PRIVATE_DEPENDS
    vtkSMTK
  EXCLUDE_FROM_WRAP_HIERARCHY
)

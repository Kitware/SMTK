set (__dependencies)

# Test for targets that might be required or
# might not exist.
foreach(target
    vtkInteractionStyle
    vtkRenderingFreeType
    vtkIOExportOpenGL2
    vtkRenderingGL2PSOpenGL2
    vtkRenderingOpenGL2
    vtkRenderingGL2PSOpenGL2
)
  if (TARGET ${target})
    list(APPEND __dependencies ${target})
  endif()
endforeach()

vtk_module(vtkSMTKDiscreteModel
  GROUPS
   DiscreteCore
  DEPENDS
    vtkCommonDataModel
    vtkIOCore
    vtkFiltersCore
    vtkRenderingCore
  PRIVATE_DEPENDS
    ${__dependencies}
    vtkFiltersSources
    vtkIOXMLParser
    vtksys
  EXCLUDE_FROM_WRAP_HIERARCHY
)

set (__dependencies vtkSMTKFilterExt) # for vtkImageSpacingFlip in vtkModelAuxiliaryGeometry

# Test for targets that might be required or
# might not exist.
foreach(target
    vtkInteractionStyle
    vtkRenderingOpenGL2
    vtkRenderingGL2PSOpenGL2
    vtkIOGDAL
)
  if (TARGET ${target})
    list(APPEND __dependencies ${target})
  endif()
endforeach()

vtk_module(vtkSMTKSourceExt
  DEPENDS
    vtkSMTKReaderExt
    vtkCommonCore
    vtkCommonDataModel
    vtkCommonExecutionModel
    vtkCommonTransforms
    vtkRenderingCore
    vtkInteractionStyle
    vtkRenderingFreeType
    vtkViewsCore
    vtkViewsInfovis
    vtkIOXML
    vtkIOLegacy
    vtkIOGeometry
  PRIVATE_DEPENDS
    ${__dependencies}
  TEST_DEPENDS
    vtkTestingRendering
  EXCLUDE_FROM_PYTHON_WRAPPING
)

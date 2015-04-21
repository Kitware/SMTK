set (__groupdeps)
set (__dependencies)

# We must test whether each of the targets below exist
# because they are required when built into VTK
# but may not be present, especially since the OpenGL2
# backend became available.
foreach(target
    vtkInteractionStyle
    vtkRenderingContext2D
    vtkRenderingContextOpenGL
    vtkRenderingOpenGL
    vtkRenderingMatplotlib
    vtkRenderingVolumeOpenGL
    vtkRenderingFreeTypeOpenGL
)
  if (TARGET ${target})
    list(APPEND __dependencies ${target})
  endif()
endforeach()

if (VTK_RENDERING_BACKEND STREQUAL "OpenGL" OR NOT VTK_RENDERING_BACKEND)
  set(__groupdeps GROUPS Rendering)
endif()

vtk_module(vtkSMTKExt
  ${__groupdeps}
  DEPENDS
    vtkCommonCore
    vtkCommonDataModel
    vtkCommonExecutionModel
    vtkRenderingCore
    vtkInteractionStyle
    vtkViewsCore
    vtkViewsInfovis
    vtkIOXML
    vtkIOLegacy
    ${__dependencies}
  TEST_DEPENDS
    vtkTestingRendering
    ${__dependencies}
  EXCLUDE_FROM_WRAP_HIERARCHY
)

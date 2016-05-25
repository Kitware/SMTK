set (__dependencies)

# Test for targets that might be required or
# might not exist.
foreach(target
    vtkInteractionStyle
    vtkRenderingFreeType
    vtkRenderingMatplotlib
    vtkRendering${VTK_RENDERING_BACKEND}
    vtkRenderingVolume${VTK_RENDERING_BACKEND}
    vtkRenderingGL2PS${VTK_RENDERING_BACKEND}
)
  if (TARGET ${target})
    list(APPEND __dependencies ${target})
  endif()
endforeach()

vtk_module(vtkSMTKWidgetsExt
  DEPENDS
    vtkInteractionWidgets
  PRIVATE_DEPENDS
    ${__dependencies}
  TEST_DEPENDS
    vtkTestingRendering
  EXCLUDE_FROM_WRAP_HIERARCHY
)

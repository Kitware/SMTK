set (__dependencies)

# Test for targets that might be required or
# might not exist.
foreach(target
    vtkInteractionStyle
    vtkRenderingFreeType
    vtkRenderingOpenGL2
    vtkRenderingMatplotlib
    vtkRenderingVolume
    vtkRenderingVolumeOpenGL2
    vtkRenderingContextOpenGL2
    vtkRenderingGL2PSOpenGL2
)
  if (TARGET ${target})
    list(APPEND __dependencies ${target})
  endif()
endforeach()
# if there is Remus, add map file reader and support files.
if(SMTK_ENABLE_REMUS_SUPPORT)
  vtk_module(vtkSMTKMeshingExt
    DEPENDS
      vtkIOXML
    PRIVATE_DEPENDS
      vtkFiltersGeneral
      vtkFiltersGeometry
      vtkGeovisCore
      vtkIOGeometry
      vtkIOParallelExodus
      vtkRenderingCore
      ${__dependencies}
      vtksys
    EXCLUDE_FROM_WRAP_HIERARCHY
  )
endif ()

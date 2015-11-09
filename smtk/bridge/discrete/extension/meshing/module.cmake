# if there is Remus, add map file reader and support files.
if(SMTK_ENABLE_REMUS_SUPPORT)
  vtk_module(vtkSMTKDiscreteMeshingExt
    DEPENDS
      vtkIOXML
    PRIVATE_DEPENDS
      vtkFiltersGeneral
      vtkFiltersGeometry
      vtkGeovisCore
      vtkIOGDAL
      vtkIOGeometry
      vtkIOParallelExodus
      vtkRenderingCore
      vtkRenderingFreeType
      vtkRendering${VTK_RENDERING_BACKEND}
      vtkRenderingMatplotlib
      vtkRenderingVolume
      vtkRenderingVolume${VTK_RENDERING_BACKEND}
      vtkRenderingContext${VTK_RENDERING_BACKEND}
      vtksys
    EXCLUDE_FROM_WRAP_HIERARCHY
  )
endif ()

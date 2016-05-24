set(_deps)
if (SMTK_ENABLE_REMUS_SUPPORT)
  list(APPEND _deps vtkSMTKMeshingExt)
endif ()

vtk_module(vtkSMTKReaderExt
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
    vtkPVVTKExtensionsDefault
    vtkRendering${VTK_RENDERING_BACKEND}
    vtkRenderingMatplotlib
    vtkRenderingVolume
    vtkRenderingVolume${VTK_RENDERING_BACKEND}
    vtkRenderingContext${VTK_RENDERING_BACKEND}
    vtksys
    ${_deps}
  EXCLUDE_FROM_WRAP_HIERARCHY
)

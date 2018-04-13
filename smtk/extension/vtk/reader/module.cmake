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
    vtkRenderingOpenGL2
    vtkRenderingVolume
    vtkRenderingVolumeOpenGL2
    vtkRenderingContextOpenGL2
    vtksys
    ${_deps}
  EXCLUDE_FROM_WRAP_HIERARCHY
)

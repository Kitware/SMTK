
set(headers
    vtkCMBUniquePointSet.h
    vtkDiscoverRegions.h
    vtkRayIntersectionLocator.h
    vtkSplitPlanarLines.h
    )

if(SMTK_ENABLE_REMUS)
  list(APPEND headers
       cmbFaceMesherInterface.h
       cmbFaceMeshHelper.h
       vtkCMBMeshServerLauncher.h
       vtkCMBPrepareForTriangleMesher.h
       vtkCMBTriangleMesher.h
       vtkPolylineTriangulator.h
       vtkRegionsToLoops.h
       )
endif()

smtk_public_headers(${headers})

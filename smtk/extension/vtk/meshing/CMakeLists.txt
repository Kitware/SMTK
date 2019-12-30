set(classes
  cmbFaceMeshHelper
  cmbFaceMesherInterface
  cmbUniquePointSet
  vtkCMBMeshServerLauncher
  vtkCMBPrepareForTriangleMesher
  vtkCMBTriangleMesher
  vtkDiscoverRegions
  vtkPolylineTriangulator
  vtkRayIntersectionLocator
  vtkRegionsToLoops
  vtkSplitPlanarLines)
set(private_headers
  union_find.h)

vtk_module_add_module(vtkSMTKMeshingExt
  CLASSES ${classes}
  PRIVATE_HEADERS ${private_headers}
  HEADERS_SUBDIR "smtk/extension/vtk/meshing")

#Remus is needed
vtk_module_link(vtkSMTKMeshingExt
  PRIVATE
    RemusClient
    RemusServer)

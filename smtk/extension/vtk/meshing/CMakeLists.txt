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
  PRIVATE_HEADERS ${private_headers})

#Remus is needed
vtk_module_link(vtkSMTKMeshingExt
  PRIVATE
    RemusClient
    RemusServer)

# The list of public headers for a VTK module are stored as a "module property"
# (which is just a namespaced CMake TARGET property with some extra support for
# being different at install time). Ask for the headers and replace any
# in-source absolute paths with relative paths.
_vtk_module_get_module_property(vtkSMTKMeshingExt
  PROPERTY headers
  VARIABLE vtk_headers)
string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" vtk_headers "${vtk_headers}")
smtk_public_headers(vtkSMTKMeshingExt ${vtk_headers})

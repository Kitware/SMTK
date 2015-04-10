PROJECT(SMTKDiscreteExtension)

set(srcs
    reader/vtkCMBSTLReader.cxx
    reader/vtkCUBITReader.cxx
    reader/vtkDataSetRegionSurfaceFilter.cxx
    reader/vtkCMBGeometryReader.cxx
    reader/vtkCMBMeshReader.cxx
    reader/vtkExtractRegionEdges.cxx
    reader/vtkGMSSolidReader.cxx
    reader/vtkGMSTINReader.cxx
    reader/vtkLIDARReader.cxx
    reader/vtkCMBReaderHelperFunctions.cxx
)

# if there is Remus, add map file reader and support files.
if(SMTK_ENABLE_REMUS)
  #Remus is needed
  find_package(Remus REQUIRED)
  list(APPEND srcs
    reader/vtkCMBGeometry2DReader.cxx
    reader/vtkCMBMapReader.cxx
    reader/vtkPolyFileReader.cxx
    meshing/cmbFaceMesherInterface.cxx
    meshing/cmbFaceMeshHelper.cxx
    meshing/vtkCMBPrepareForTriangleMesher.cxx
    meshing/vtkCMBMeshServerLauncher.cxx
    meshing/vtkDiscoverRegions.cxx
    meshing/vtkPolylineTriangulator.cxx
    meshing/vtkCMBTriangleMesher.cxx
    meshing/vtkRegionsToLoops.cxx
    meshing/vtkSplitPlanarLines.cxx
    meshing/vtkRayIntersectionLocator.cxx
    )
endif()

# set(Meshing_Libs)
# if(BUILD_MESH_WORKERS)
#  list(APPEND Meshing_Libs
#       omicron
#       triangle_omicron
#       tl_alloc
#       )
# endif()

# no wrapping for sources
set_source_files_properties(${srcs} WRAP_EXCLUDE)

set(_VTK_INSTALL_NO_DEVELOPMENT ${VTK_INSTALL_NO_DEVELOPMENT})
set(VTK_INSTALL_NO_DEVELOPMENT ON)
set(vtkSMTKDiscreteExt_NO_HeaderTest 1)

vtk_module_library(vtkSMTKDiscreteExt ${srcs})
SET(VTK_INSTALL_NO_DEVELOPMENT ${_VTK_INSTALL_NO_DEVELOPMENT})

if(SMTK_ENABLE_REMUS)
  #Remus is needed
  target_link_libraries(vtkSMTKDiscreteExt LINK_PRIVATE RemusClient RemusServer)
  target_include_directories(vtkSMTKDiscreteExt PRIVATE ${REMUS_INCLUDE_DIRS})
endif()


# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(vtkSMTKDiscreteExt PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# ... and make header compilation tests link properly:
smtk_install_library(vtkSMTKDiscreteExt)

# ... and install the export header
smtk_get_kit_name(name dir_prefix)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vtkSMTKDiscreteExtModule.h  DESTINATION include/${dir_prefix})

#include the subdirectories so that the headers are installed. We don't invoke
#smtk_public_headers from this directory as that would cause the headers to
#be installed in the wrong location
add_subdirectory(reader)
add_subdirectory(meshing)

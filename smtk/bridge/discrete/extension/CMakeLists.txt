PROJECT(SMTKDiscreteExtension)

set(EXT_READER_SRC
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
set(EXT_READER_HEADER
    reader/vtkCMBSTLReader.h
    reader/vtkCUBITReader.h
    reader/vtkDataSetRegionSurfaceFilter.h
    reader/vtkCMBGeometryReader.h
    reader/vtkCMBMeshReader.h
    reader/vtkExtractRegionEdges.h
    reader/vtkGMSSolidReader.h
    reader/vtkGMSTINReader.h
    reader/vtkLIDARReader.h
    reader/vtkCMBReaderHelperFunctions.h
)

# if there is Remus, add map file reader and support files.
set(Readers_Need_Meshing_SRC)
set(Readers_Need_Meshing_HEADER)
if(SMTK_ENABLE_REMUS)

message( "Adding Meshing related files ..." )

  #Remus is needed
  find_package(Remus REQUIRED)
  list(APPEND Readers_Need_Meshing_SRC
    reader/vtkCMBMapReader.cxx
    reader/vtkPolyFileReader.cxx
    meshing/cmbFaceMesherInterface.cxx
    meshing/cmbFaceMeshHelper.cxx
    meshing/vtkCMBPrepareForTriangleMesher.cxx
    meshing/vtkCMBMeshServerLauncher.cxx
    meshing/vtkDiscoverRegions.cxx
    meshing/vtkPolylineTriangulator.cxx
    meshing/vtkCMBTriangleMesher.cxx
    )
  list(APPEND Readers_Need_Meshing_HEADER
    reader/vtkCMBMapReader.h
    reader/vtkPolyFileReader.h
    reader/vtkPolyFileErrorReporter.h
    reader/vtkPolyFileTokenConverters.h
    meshing/cmbFaceMesherInterface.h
    meshing/cmbFaceMeshHelper.h
    meshing/vtkCMBPrepareForTriangleMesher.h
    meshing/vtkCMBMeshServerLauncher.h
    meshing/vtkDiscoverRegions.h
    meshing/vtkPolylineTriangulator.h
    meshing/vtkCMBUniquePointSet.h
    meshing/vtkCMBTriangleMesher.h
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
set_source_files_properties(
  ${Readers_Need_Meshing_SRC}
  ${EXT_READER_SRC}
  WRAP_EXCLUDE
)

set(meshing_Depend_LIBS)
if(SMTK_ENABLE_REMUS)
  #Remus is needed
  find_package(Remus REQUIRED)
  list(APPEND meshing_Depend_LIBS
    RemusClient
    RemusServer
  )
endif()

set(vtkSMTKDiscreteExt_NO_HeaderTest 1)
vtk_module_library(vtkSMTKDiscreteExt
  ${Readers_Need_Meshing_SRC}
  ${EXT_READER_SRC}
)

target_link_libraries(vtkSMTKDiscreteExt LINK_PRIVATE ${meshing_Depend_LIBS})

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(vtkSMTKDiscreteExt PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# install the headers
smtk_public_headers(${EXT_READER_HEADER} ${Readers_Need_Meshing_HEADER})

# ... and make header compilation tests link properly:
smtk_install_library(vtkSMTKDiscreteExt)


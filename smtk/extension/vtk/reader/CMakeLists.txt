# Required for support of shapefiles, which is required to construct
# the gdal wrapper classes needed by cmb.
find_package(GDAL REQUIRED)

set(classes
  vtkCMBGeometryReader
  vtkCMBMeshReader
  vtkCMBSTLReader
  vtkCUBITReader
  vtkExtractRegionEdges
  vtkGDALRasterPolydataWrapper
  vtkGMSSolidReader
  vtkGMSTINReader
  vtkGeoSphereTransformLegacy
  vtkGlobeSourceLegacy
  vtkLIDARReader
  vtkSMTKLASReader)
set(private_headers
  vtkCMBReaderHelperFunctions.h)

# if there is Remus, add map file reader and support files.
if(SMTK_ENABLE_REMUS_SUPPORT)
  #Remus is needed
  find_package(Remus REQUIRED)
  list(APPEND classes
    vtkCMBGeometry2DReader
    vtkCMBMapReader
    vtkCMBPolygonModelImporter
    vtkPolyFileReader)
  list(APPEND private_headers
    vtkPolyFileErrorReporter.h
    vtkPolyFileTokenConverters.h)
endif()

vtk_module_add_module(vtkSMTKReaderExt
  CLASSES ${classes}
  PRIVATE_HEADERS ${private_headers}
  HEADERS_SUBDIR "smtk/extension/vtk/reader")

if(SMTK_ENABLE_REMUS_SUPPORT)
  #Remus is needed
  vtk_module_link(vtkSMTKReaderExt
    PRIVATE
      RemusClient
      RemusServer)
endif()

# Link GDAL and Add a compiler definition to the target
# TODO: Use imported target.
vtk_module_link(vtkSMTKReaderExt PRIVATE ${GDAL_LIBRARY} )
vtk_module_include(vtkSMTKReaderExt PRIVATE ${GDAL_INCLUDE_DIR})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

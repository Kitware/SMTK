set(headers
  vtkCMBSTLReader.h
  vtkCUBITReader.h
  vtkDataSetRegionSurfaceFilter.h
  vtkCMBGeometryReader.h
  vtkCMBMeshReader.h
  vtkExtractRegionEdges.h
  vtkGMSSolidReader.h
  vtkGMSTINReader.h
  vtkLIDARReader.h
  vtkCMBReaderHelperFunctions.h
  vtkLASReader.h
)

if (SMTK_ENABLE_REMUS_SUPPORT)
  list(APPEND headers
    vtkCMBGeometry2DReader.h
    vtkCMBMapReader.h
    vtkPolyFileReader.h
    vtkPolyFileErrorReporter.h
    vtkPolyFileTokenConverters.h
  )
endif()

if(GDAL_FOUND)
  list(APPEND headers
    vtkGDALRasterReader.h
    vtkGDALRasterPolydataWrapper.h
  )
endif()

smtk_public_headers(${headers})

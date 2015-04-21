
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
    )

if(SMTK_ENABLE_REMUS)
  list(APPEND headers
    vtkCMBGeometry2DReader.h
    vtkCMBMapReader.h
    vtkPolyFileReader.h
    vtkPolyFileErrorReporter.h
    vtkPolyFileTokenConverters.h
    )
endif()

smtk_public_headers(${headers})

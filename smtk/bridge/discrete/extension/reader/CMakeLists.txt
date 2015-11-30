PROJECT(SMTKDiscreteExtensionReader)

set(srcs
    vtkCMBSTLReader.cxx
    vtkCUBITReader.cxx
    vtkCMBGeometryReader.cxx
    vtkCMBMeshReader.cxx
    vtkExtractRegionEdges.cxx
    vtkGMSSolidReader.cxx
    vtkGMSTINReader.cxx
    vtkLIDARReader.cxx
    vtkCMBReaderHelperFunctions.cxx
    vtkLASReader.cxx
)

set(headers
  vtkCMBSTLReader.h
  vtkCUBITReader.h
  vtkCMBGeometryReader.h
  vtkCMBMeshReader.h
  vtkExtractRegionEdges.h
  vtkGMSSolidReader.h
  vtkGMSTINReader.h
  vtkLIDARReader.h
  vtkCMBReaderHelperFunctions.h
  vtkLASReader.h
)

# if there is Remus, add map file reader and support files.
if(SMTK_ENABLE_REMUS_SUPPORT)
  #Remus is needed
  find_package(Remus REQUIRED)
  list(APPEND srcs
    vtkCMBGeometry2DReader.cxx
    vtkCMBMapReader.cxx
    vtkPolyFileReader.cxx
    )
  list(APPEND headers
    vtkCMBGeometry2DReader.h
    vtkCMBMapReader.h
    vtkPolyFileReader.h
    vtkPolyFileErrorReporter.h
    vtkPolyFileTokenConverters.h
  )
endif()

if(GDAL_FOUND)
  list(APPEND srcs
    vtkGDALRasterPolydataWrapper.cxx
  )
  list(APPEND headers
    vtkGDALRasterPolydataWrapper.h
  )
endif()

# no wrapping for sources
set_source_files_properties(
  vtkCMBReaderHelperFunctions.cxx
  PROPERTIES
    WRAP_EXCLUDE
    WRAP_EXCLUDE_PYTHON)

set(${vtk-module}_NO_HeaderTest 1)
set(VTK_INSTALL_NO_HEADERS 1)
vtk_module_library(${vtk-module} ${srcs})

if(SMTK_ENABLE_REMUS_SUPPORT)
  #Remus is needed
  target_link_libraries(${vtk-module} LINK_PRIVATE RemusClient RemusServer)
  target_include_directories(${vtk-module} PRIVATE ${REMUS_INCLUDE_DIRS})
endif()


# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(${vtk-module} PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# Link GDAL and Add a compiler definition to the target
if(GDAL_FOUND)
  target_link_libraries(${vtk-module} LINK_PRIVATE ${GDAL_LIBRARY} )
  target_include_directories(${vtk-module} PRIVATE ${GDAL_INCLUDE_DIR})

  set_property(TARGET ${vtk-module} APPEND PROPERTY COMPILE_DEFINITIONS HAS_GDAL_RASTER_READER)
endif()

# ... and make header compilation tests link properly:
smtk_install_library(${vtk-module})

# ... and install the export header
smtk_get_kit_name(name dir_prefix)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${vtk-module}Module.h  DESTINATION include/smtk/${SMTK_VERSION}/${dir_prefix})

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  vtk_add_cs_wrapping(${vtk-module})
  smtk_install_library(${vtk-module}CS)
endif ()

smtk_public_headers(${headers})

PROJECT(SMTKVTKExtensionMeshing)

set(srcs
  cmbFaceMesherInterface.cxx
  cmbFaceMeshHelper.cxx
  vtkCMBPrepareForTriangleMesher.cxx
  vtkCMBMeshServerLauncher.cxx
  vtkDiscoverRegions.cxx
  vtkPolylineTriangulator.cxx
  vtkCMBTriangleMesher.cxx
  vtkRegionsToLoops.cxx
  vtkSplitPlanarLines.cxx
  vtkRayIntersectionLocator.cxx
  )

set(headers
  vtkCMBUniquePointSet.h
  vtkDiscoverRegions.h
  vtkRayIntersectionLocator.h
  vtkSplitPlanarLines.h
  cmbFaceMesherInterface.h
  cmbFaceMeshHelper.h
  vtkCMBMeshServerLauncher.h
  vtkCMBPrepareForTriangleMesher.h
  vtkCMBTriangleMesher.h
  vtkPolylineTriangulator.h
  vtkRegionsToLoops.h
)

# no wrapping for sources
set_source_files_properties(
  cmbFaceMesherInterface.cxx
  cmbFaceMeshHelper.cxx
  PROPERTIES
    WRAP_EXCLUDE
    WRAP_EXCLUDE_PYTHON)

set(${vtk-module}_NO_HeaderTest 1)
set(VTK_INSTALL_NO_HEADERS 1)
vtk_module_library(${vtk-module} ${srcs})
target_include_directories(${vtk-module}
  PUBLIC
    $<BUILD_INTERFACE:${SMTK_SOURCE_DIR}>
    $<BUILD_INTERFACE:${SMTK_BINARY_DIR}>
    $<INSTALL_INTERFACE:include/smtk/${SMTK_VERSION}>)

if(SMTK_ENABLE_REMUS_SUPPORT)
  #Remus is needed
  target_link_libraries(${vtk-module} LINK_PRIVATE RemusClient RemusServer)
  target_include_directories(${vtk-module} PRIVATE ${REMUS_INCLUDE_DIRS})
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
#  vtk_add_cs_wrapping(${vtk-module})
#  smtk_install_library(${vtk-module}CS)
endif ()

smtk_public_headers(${headers})

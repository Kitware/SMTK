CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2 FATAL_ERROR)

project(DiscreteMoabReader)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

find_package(MOAB REQUIRED)

option ( MOAB_HAS_CGM        "Moab was built with CGM support?"       OFF )
option ( MOAB_HAS_HDF        "Moab was built with HDF support?"       ON )
option ( MOAB_HAS_NETCDF     "Moab was built with NETCDF support?"    ON )

set(MOAB_LIBS ${MOAB_LIBRARIES})
if ( MOAB_HAS_NETCDF )
  find_package( NetCDF REQUIRED )
  include_directories( ${NetCDF_INCLUDE_DIRECTORIES} )
  set( MOAB_LIBS ${MOAB_LIBS} ${NETCDF_LIBRARIES} )
endif ( )

if ( MOAB_HAS_HDF )
  find_package( HDF5 REQUIRED )
  include_directories( ${HDF5_INCLUDE_DIRS} )
  set( MOAB_LIBS ${MOAB_LIBS} ${HDF5_LIBRARIES} )
endif ( )

if ( MOAB_HAS_CGM )
   find_package( CGM REQUIRED )
   include_directories( ${CGM_INCLUDE_DIRS} )
   set( MOAB_LIBS ${MOAB_LIBS} ${CGM_LIBRARIES} )
endif ()

include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/detail"
    "${CMAKE_CURRENT_SOURCE_DIR}/model"
    "${CMAKE_CURRENT_BINARY_DIR}/detail"
    "${CMAKE_CURRENT_BINARY_DIR}/model"
)

set(headers
  CellSets.h
  DataSetConverter.h
  detail/CellTypeToType.h
  detail/ContinousCellInfo.h
  detail/LoadGeometry.h
  detail/MixedCellConnectivity.h
  detail/ReadSparseTag.h
  detail/ReduceSpectralMesh.h
  detail/UsageTable.h
  ExtractShell.h
  model/BoundaryConditions.h
  model/FaceSets.h
  SimpleMoab.h
  vtkCmbMoabReader.h
  )

add_library(vtkDiscreteMoabReader vtkCmbMoabReader.cxx)

#link agianst the discrete Model library and moab
target_link_libraries(vtkDiscreteMoabReader
                      LINK_PUBLIC
                      ${MOAB_LIBS}
                      vtkSMTKDiscreteModel
                      )

smtk_export_header(vtkDiscreteMoabReader discreteMoabExports.h)

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(vtkDiscreteMoabReader PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# Install the headers.
smtk_public_headers(vtkCmbMoabReader.h)

#install the library and exports the library when used from a build tree
smtk_install_library(vtkDiscreteMoabReader DEPENDS vtkSMTKDiscreteModel)

target_include_directories(vtkDiscreteMoabReader
                          PUBLIC ${MOAB_INCLUDE_DIRS})

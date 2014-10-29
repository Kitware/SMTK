CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2 FATAL_ERROR)

project(vtkCmbMoabReader)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

find_package(MOAB REQUIRED)
find_package(CMB REQUIRED)

option ( MOAB_HAS_CGM        "Moab was built with CGM support?"       ON )
option ( MOAB_HAS_HDF        "Moab was built with HDF support?"       ON )
option ( MOAB_HAS_NETCDF     "Moab was built with NETCDF support?"    ON )

set(MOAB_LIBS ${MOAB_LIBRARIES})
if ( MOAB_HAS_NETCDF )
  find_package( NetCDF REQUIRED )
  include_directories( ${NetCDF_INCLUDE_DIRECTORIES} )
  set( MOAB_LIBS ${MOAB_LIBS} ${NetCDF_LIBRARIES} )
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
  )

add_paraview_plugin(vtkCmbMoabReaderPlugin "5.0"
   SERVER_MANAGER_XML vtkCmbMoabReader.xml
   SERVER_MANAGER_SOURCES vtkCmbMoabReader.cxx
   SERVER_SOURCES ${headers}
   REQUIRED_ON_SERVER)

#link agianst the VTKExtension and Model library and moab
target_link_libraries(vtkCmbMoabReaderPlugin
                      LINK_PUBLIC
                      ${MOAB_LIBS}
                      vtkCmbDiscreteModel
                      vtkCMBIO
                      )
target_include_directories(vtkCmbMoabReaderPlugin
                          PUBLIC ${MOAB_INCLUDE_DIRS})

#install the plugin
cmb_install_plugin(vtkCmbMoabReaderPlugin)
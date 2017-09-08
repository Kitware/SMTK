include(${PARAVIEW_USE_FILE})
include(ParaViewPlugins)

# set up sources to build
set(serverUnwrappedSrcs
  smtkModelEntityPointLocator.cxx
)

set(serverUnwrappedHdrs
  smtkModelEntityPointLocator.h
)

set(serverXMLSrcs
  smconfig.xml
)

set(serverWrappedSrcs
  vtkPVModelSources.cxx
)

set(serverWrappedHdrs
  vtkPVModelSources.h
)

set(hdrs ${serverUnwrappedHdrs} ${serverWrappedHdrs})

#install the headers
smtk_public_headers(${hdrs})

#target_include_directories(smtkPVServerExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Generate a ParaView plugin
add_paraview_plugin(
  smtkPVServerExtPlugin "1.0"
  REQUIRED_ON_SERVER
  SERVER_SOURCES ${serverUnwrappedSrcs}
  SERVER_MANAGER_SOURCES ${serverWrappedSrcs}
  SERVER_MANAGER_XML ${serverXMLSrcs}
)
target_link_libraries(smtkPVServerExtPlugin
  LINK_PUBLIC
    smtkCore
    vtkSMTKSourceExt
    vtkSMTKReaderExt # vtkLIDARReader
    vtkSMTKFilterExt # vtkTerrainExtractionFilter, vtkLIDARElevationFilter
)
smtk_export_header(smtkPVServerExtPlugin Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPVServerExtPlugin)

#if (SMTK_ENABLE_TESTING)
#  target_link_libraries(TestBuild_smtk_extension_paraview_operators
#    LINK_PUBLIC
#      smtkPVServerExtPlugin
#  )
#endif()


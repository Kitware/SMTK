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
  vtkSMTKCompositeRepresentation.cxx
  vtkSMTKResource.cxx
  vtkSMTKResourceCreator.cxx
  vtkSMTKResourceGenerator.cxx
  vtkSMTKResourceImporter.cxx
  vtkSMTKResourceReader.cxx
  vtkSMTKResourceSource.cxx
  vtkSMTKSelectionRepresentation.cxx
  vtkSMTKSettings.cxx
  vtkSMSMTKWrapperProxy.cxx
  vtkSMTKWrapper.cxx
  vtkSMSMTKResourceRepresentationProxy.cxx
  vtkSMTKResourceRepresentation.cxx
)

set(serverWrappedHdrs
  vtkPVModelSources.h
  vtkSMTKCompositeRepresentation.h
  vtkSMTKResource.h
  vtkSMTKResourceCreator.h
  vtkSMTKResourceGenerator.h
  vtkSMTKResourceImporter.h
  vtkSMTKResourceReader.h
  vtkSMTKResourceSource.h
  vtkSMTKSelectionRepresentation.h
  vtkSMTKSettings.h
  vtkSMSMTKWrapperProxy.h
  vtkSMTKWrapper.h
  vtkSMSMTKResourceRepresentationProxy.h
  vtkSMTKResourceRepresentation.h
)

# the server plugin adds vtkCMBGeometryReader as a source, so we need the
# vtk module that contains it (vtkSMTKReaderExt) listed as add_paraview_plugin's
# CS_KITS parameter. We also must link against the client-server wrapping
# library for this module (vtkSMTKReaderExtCS).
set(serverModules
  vtkSMTKReaderExt
  vtkSMTKSourceExt
  vtkSMTKWidgetsExt
  )

set(serverModuleCSLibs)
foreach (cslib IN LISTS serverModules)
  list(APPEND serverModuleCSLibs ${cslib}CS)
endforeach ()

set(hdrs ${serverUnwrappedHdrs} ${serverWrappedHdrs})

add_library(smtkPVServerExt
  ${serverUnwrappedSrcs}
  ${serverWrappedSrcs}
)
target_link_libraries(smtkPVServerExt
  LINK_PUBLIC
    smtkCore
    smtkPluginSupport
    vtkPVClientServer
    vtkPVServerManager
    vtkSMTKSourceExt
    vtkSMTKReaderExt # vtkLIDARReader
    vtkSMTKFilterExt # vtkTerrainExtractionFilter, vtkLIDARElevationFilter
    ${serverModuleCSLibs}
)
target_include_directories(smtkPVServerExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Generate a ParaView plugin
add_paraview_plugin(
  smtkPVServerExtPlugin "1.0"
  REQUIRED_ON_SERVER
  REQUIRED_ON_CLIENT
  SERVER_MANAGER_SOURCES ${serverWrappedHdrs}
  SERVER_MANAGER_XML ${serverXMLSrcs}
  CS_KITS ${serverModules}
)
set_property(GLOBAL APPEND PROPERTY SMTK_PLUGINS "smtkPVServerExtPlugin")
enable_smtk_plugin_by_default(smtkPVServerExtPlugin TRUE)

target_link_libraries(smtkPVServerExtPlugin
  LINK_PUBLIC
    smtkPVServerExt
)
smtk_export_header(smtkPVServerExt Exports.h)

# Install the library and exports the library when used from a build tree
smtk_install_library(smtkPVServerExt)
smtk_install_library(smtkPVServerExtPlugin)
# Install the headers
smtk_public_headers(smtkPVServerExt ${hdrs})

add_subdirectory(testing)

include(${PARAVIEW_USE_FILE})
include(ParaViewPlugins)

set(REP_SOURCE
  vtkSMSMTKModelRepresentationProxy.cxx
  vtkSMTKModelRepresentation.cxx
)

set(REP_HEADER
  vtkSMSMTKModelRepresentationProxy.h
  vtkSMTKModelRepresentation.h
)

set(REP_XML
  representation.xml
)

# Install the headers
smtk_public_headers(${REP_HEADER})

# Generate a ParaView plugin
add_paraview_plugin(
  smtkRepresentationPlugin "1.0"
  SERVER_MANAGER_SOURCES ${REP_SOURCE}
  SERVER_MANAGER_XML ${REP_XML}
)

target_link_libraries(smtkRepresentationPlugin
  LINK_PUBLIC
    smtkCore
    vtkSMTKSourceExt
    smtkPVServerExtPlugin
)

smtk_export_header(smtkRepresentationPlugin Exports.h)

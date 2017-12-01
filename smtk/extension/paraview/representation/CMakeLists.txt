include(${PARAVIEW_USE_FILE})
include(ParaViewPlugins)

find_package(Qt5 REQUIRED COMPONENTS Core)

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

set(REP_PXY_HDRS
  pqSMTKModelRepresentation.h
)
set(REP_PXY_SRCS
  pqSMTKModelRepresentation.cxx
)

# Install the headers
smtk_public_headers(${REP_HEADER})

add_pqproxy(REP_PXY_IFACES REP_PXY_IFACE_SRCS
  TYPE pqSMTKModelRepresentation
  XML_GROUP representations
  XML_NAME SMTKModelRepresentation
)

pv_qt_wrap_cpp(MOC_BUILT_SRCS ${REP_PXY_HDRS})

# Generate a ParaView plugin
add_paraview_plugin(
  smtkRepresentationPlugin "1.0"
  GUI_INTERFACES ${REP_PXY_IFACES}
  GUI_SOURCES
    ${REP_PXY_SRCS}
    ${REP_PXY_HDRS}
    ${MOC_BUILT_SRCS}
    ${REP_PXY_IFACE_SRCS}
  SERVER_MANAGER_SOURCES ${REP_SOURCE}
  SERVER_MANAGER_XML ${REP_XML}
)

target_link_libraries(smtkRepresentationPlugin
  LINK_PUBLIC
    smtkCore
    vtkSMTKSourceExt
    smtkPVServerExtPlugin
    smtkPQComponentsExt
    Qt5::Core
)

smtk_export_header(smtkRepresentationPlugin Exports.h)

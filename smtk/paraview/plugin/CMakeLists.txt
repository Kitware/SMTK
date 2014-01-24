include_directories(
  ${vtkSMTK_SOURCE_DIR}
  ${pvSMTK_SOURCE_DIR}
  )

qt4_wrap_cpp(MOC_SRCS
  pqSMTKModelReaderPanel.h
)

set(PVLibUIs
  pqSMTKModelReaderPanel.cxx
  )

add_paraview_object_panel(IFACES1 IFACE_SRCS1
  CLASS_NAME pqSMTKModelReaderPanel
  XML_NAME SMTKModelReader
  XML_GROUP sources
)

set(PVLibSrcs
  vtkPVSMTKModelInformation.cxx
  )

set(PVLibHeaders
  vtkPVSMTKModelInformation.h
  pqSMTKModelReaderPanel.h
  )

source_group("pv_Source" FILES ${PVLibSrcs} ${PVLibUIs})
source_group("pv_Header" FILES ${PVLibHeaders})

set(pvsmtk_headers ${PVLibHeaders})
set(pvsmtk_srcs ${PVLibSrcs})

add_paraview_plugin(SMTKModel_Plugin "1.0"
  SERVER_MANAGER_XML smSMTKModel.xml
  GUI_INTERFACES
    ${IFACES1}
  SERVER_MANAGER_SOURCES
    ${PVLibSrcs}
  GUI_SOURCES
    ${PVLibUIs}
    ${IFACE_SRCS1}
    ${MOC_SRCS}
  CS_KITS
    vtkSMTK
    pvSMTK
)

target_link_libraries(SMTKModel_Plugin
  LINK_PUBLIC
    SMTKCore
    QtSMTK
    vtkSMTK
    pvSMTK
  LINK_PRIVATE
    pvSMTKCS
    vtkSMTKCS
  )

# smtk_install_library(SMTKModel_Plugin)
# smtk_install_plugin(SMTKModel_Plugin)

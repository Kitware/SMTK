###
# This directory contains 2 targets:
# + smtkPQComponentsExt, containing non-Qt (unwrapped) classes and Qt classes
# + smtkPQComponentsPlugin, a plugin exposing the above with an autostart object
###

set(PQComponentsUnwrappedSrcs
)

set(PQComponentsUnwrappedHdrs
)

find_package(Qt5 REQUIRED COMPONENTS Core)

# set up sources to build
set(PQComponentsSrcs
  pqPluginSMTKViewBehavior.cxx
  pqSMTKBehavior.cxx
  pqSMTKColorByToolBar.cxx
  pqSMTKColorByWidget.cxx
  pqSMTKModelRepresentation.cxx
  pqSMTKSelectionFilterBehavior.cxx
  pqSMTKResource.cxx
  pqSMTKWrapper.cxx
  pqSMTKResourcePanel.cxx
)

set(PQComponentsHeaders
  pqPluginSMTKViewBehavior.h
  pqSMTKBehavior.h
  pqSMTKColorByToolBar.h
  pqSMTKColorByWidget.h
  pqSMTKModelRepresentation.h
  pqSMTKSelectionFilterBehavior.h
  pqSMTKResource.h
  pqSMTKWrapper.h
  pqSMTKResourcePanel.h
)

set(PQComponentsUIs
  resources/pqSMTKSelectionFilterBehavior.ui
  resources/pqSMTKResourcePanel.ui
  resources/pqSMTKColorByWidget.ui
)

set(PQComponentsResources
  resources/pqSMTKSelectionFilterBehavior.qrc
)

#install the headers
smtk_public_headers(
  ${PQComponentsHeaders}
  ${PQComponentsUnwrappedHdrs}
)

pv_qt_wrap_cpp(MOC_BUILT_SOURCES ${PQComponentsHeaders})
pv_qt_wrap_ui(UI_BUILT_SOURCES ${PQComponentsUIs})
pv_qt_add_resources(RCS_SOURCES ${PQComponentsResources})

source_group("Qt_Source" FILES ${PQComponentsSrcs} )
source_group("Qt_Generated" FILES ${MOC_BUILT_SOURCES} )
source_group("Qt_Header" FILES ${PQComponentsHeaders})
source_group("Qt_Resources" FILES ${PQComponentsUIs})

add_library(smtkPQComponentsExt
  ${PQComponentsSrcs}
  ${PQComponentsUnwrappedSrcs}
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${RCS_SOURCES}
)

# Need to add the location of the moc files to the include dirs:
target_include_directories(smtkPQComponentsExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#set smtkPQComponentsExt to publicly link to smtkCore
target_link_libraries(smtkPQComponentsExt LINK_PUBLIC
  pqApplicationComponents
  pqComponents

  # Server-side stuff because some stuff uses GetClientSideObject()
  vtkPVVTKExtensionsDefault
  smtkPVServerExtPlugin

  smtkQtExt
  smtkOperationEnvironment
  smtkEnvironment
  Qt5::Core
)

if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_extension_paraview_appcomponents
    LINK_PUBLIC
    smtkPQComponentsExt
    )
endif()

smtk_export_header(smtkPQComponentsExt Exports.h)

set(PQComponentsPluginHdrs
  pqSMTKAppComponentsAutoStart.h
)

set(PQComponentsPluginSrcs
  pqSMTKAppComponentsAutoStart.cxx
)

add_paraview_auto_start(PLUGIN_AUTOSTART_IFACES PLUGIN_AUTOSTART_IFACE_SRCS
  CLASS_NAME pqSMTKAppComponentsAutoStart
  STARTUP startup
  SHUTDOWN shutdown
)

add_paraview_action_group(selectionFilterIfaces selectionFilterIfaceSrcs
  CLASS_NAME pqSMTKSelectionFilterBehavior
  GROUP_NAME "ToolBar/SMTK Selection Filters"
)
list(APPEND PLUGIN_ACTION_IFACES "${selectionFilterIfaces}")
list(APPEND PLUGIN_ACTION_IFACE_SRCS "${selectionFilterIfaceSrcs}")

add_paraview_toolbar(colorByToolBarIfaces colorByToolBarIfaceSrcs
  CLASS_NAME pqSMTKColorByToolBar
)
list(APPEND PLUGIN_ACTION_IFACES "${colorByToolBarIfaces}")
list(APPEND PLUGIN_ACTION_IFACE_SRCS "${colorByToolBarIfaceSrcs}")

add_pqproxy(modelRepIfaces modelRepIfaceSrcs
  TYPE pqSMTKModelRepresentation
  XML_GROUP representations
  XML_NAME SMTKModelCompositeRepresentation
)
list(APPEND PLUGIN_PROXY_IFACES ${modelRepIfaces})
list(APPEND PLUGIN_PROXY_IFACE_SRCS ${modelRepIfaceSrcs})

add_pqproxy(rsrcIfaces rsrcIfaceSrcs
  TYPE pqSMTKResource
  XML_GROUP sources
  XML_NAME SMTKModelReader
)
list(APPEND PLUGIN_PROXY_IFACES ${rsrcIfaces})
list(APPEND PLUGIN_PROXY_IFACE_SRCS ${rsrcIfaceSrcs})

add_pqproxy(wrapperIfaces wrapperIfaceSrcs
  TYPE pqSMTKWrapper
  XML_GROUP smtk
  XML_NAME SMTKWrapper
)
list(APPEND PLUGIN_PROXY_IFACES ${wrapperIfaces})
list(APPEND PLUGIN_PROXY_IFACE_SRCS ${wrapperIfaceSrcs})

add_paraview_dock_window(PLUGIN_DOCK_IFACES PLUGIN_DOCK_IFACES_SRCS
  CLASS_NAME pqSMTKResourcePanel
  DOCK_AREA Left
)

pv_qt_wrap_cpp(PLUGIN_MOC_SRCS ${PQComponentsPluginHdrs})
add_paraview_plugin(
  smtkPQComponentsPlugin "1.0"
  GUI_INTERFACES
    ${PLUGIN_ACTION_IFACES}
    ${PLUGIN_AUTOSTART_IFACES}
    ${PLUGIN_PROXY_IFACES}
    ${PLUGIN_DOCK_IFACES}
  GUI_SOURCES
    ${PQComponentsPluginHdrs}
    ${PQComponentsPluginSrcs}
    ${PLUGIN_MOC_SRCS}
    ${PLUGIN_ACTION_IFACE_SRCS}
    ${PLUGIN_AUTOSTART_IFACE_SRCS}
    ${PLUGIN_PROXY_IFACE_SRCS}
    ${PLUGIN_DOCK_IFACES_SRCS}
  REQUIRED_PLUGINS
    smtkPVServerExtPlugin
)
target_link_libraries(smtkPQComponentsPlugin
  LINK_PUBLIC
    smtkPQComponentsExt
)
smtk_export_header(smtkPQComponentsPlugin PluginExports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPQComponentsExt)
smtk_install_library(smtkPQComponentsPlugin)

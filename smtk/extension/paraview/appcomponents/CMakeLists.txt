find_package(Qt5 REQUIRED COMPONENTS Core)

# set up sources to build
set(PQComponentsSrcs
  pqPluginSMTKViewBehavior.cxx
  pqSMTKSelectionFilterBehavior.cxx
  pqSMTKSelectionSyncBehavior.cxx
)

set(PQComponentsHeaders
  pqPluginSMTKViewBehavior.h
  pqSMTKSelectionFilterBehavior.h
  pqSMTKSelectionSyncBehavior.h
)

set(PQComponentsUIs
  resources/pqSMTKSelectionFilterBehavior.ui
)

set(PQComponentsResources
  resources/pqSMTKSelectionFilterBehavior.qrc
)

#install the headers
smtk_public_headers(${PQComponentsHeaders})

pv_qt_wrap_cpp(MOC_BUILT_SOURCES ${PQComponentsHeaders})
pv_qt_wrap_ui(UI_BUILT_SOURCES ${PQComponentsUIs})
pv_qt_add_resources(RCS_SOURCES ${PQComponentsResources})

source_group("Qt_Source" FILES ${PQComponentsSrcs} )
source_group("Qt_Generated" FILES ${MOC_BUILT_SOURCES} )
source_group("Qt_Header" FILES ${PQComponentsHeaders})
source_group("Qt_Resources" FILES ${PQComponentsUIs})

add_library(smtkPQComponentsExt
  ${PQComponentsSrcs}
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

  # Server-side stuff because pqSMTKSelectionSyncBehavior uses GetClientSideObject()
  vtkPVVTKExtensionsDefault
  smtkPVServerExtPlugin

  smtkQtExt
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

add_paraview_action_group(PLUGIN_ACTION_IFACES PLUGIN_ACTION_IFACE_SRCS
  CLASS_NAME pqSMTKSelectionFilterBehavior
  GROUP_NAME "ToolBar/SMTKSelection"
)

pv_qt_wrap_cpp(PLUGIN_MOC_SRCS ${PQComponentsPluginHdrs})
add_paraview_plugin(
  smtkPQComponentsPlugin "1.0"
  GUI_INTERFACES
    ${PLUGIN_ACTION_IFACES}
    ${PLUGIN_AUTOSTART_IFACES}
  GUI_SOURCES
    ${PQComponentsPluginHdrs}
    ${PQComponentsPluginSrcs}
    ${PLUGIN_MOC_SRCS}
    ${PLUGIN_ACTION_IFACE_SRCS}
    ${PLUGIN_AUTOSTART_IFACE_SRCS}
)
target_link_libraries(smtkPQComponentsPlugin
  LINK_PUBLIC
    smtkPQComponentsExt
)
smtk_export_header(smtkPQComponentsPlugin PluginExports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPQComponentsExt)
smtk_install_library(smtkPQComponentsPlugin)

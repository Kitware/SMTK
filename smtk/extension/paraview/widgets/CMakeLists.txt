find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# set up sources to build
set(PQWidgetsSrcs
  pqGenerateContoursDialog.cxx
  pqSMTKAttributeItemWidget.cxx
  pqSMTKBoxItemWidget.cxx
  pqSMTKLineItemWidget.cxx
  pqSMTKPlaneItemWidget.cxx
  pqSMTKPointItemWidget.cxx
  pqSMTKSphereItemWidget.cxx
  pqSMTKSplineItemWidget.cxx
  qtArcWidget.cxx
  qtInteractionWidget.cxx
)

set(PQWidgetsUIs
  qtArcWidget.ui
  qtGenerateContoursDialog.ui
)

set(PQWidgetsMocHeaders
  pqGenerateContoursDialog.h
  pqSMTKAttributeItemWidget.h
  pqSMTKBoxItemWidget.h
  pqSMTKLineItemWidget.h
  pqSMTKPlaneItemWidget.h
  pqSMTKPointItemWidget.h
  pqSMTKSphereItemWidget.h
  pqSMTKSplineItemWidget.h
  qtArcWidget.h
  qtInteractionWidget.h
)

set(PQWidgetsHeaders
  ${PQWidgetsMocHeaders}
  pqSMTKAttributeItemWidgetP.h
)

#install the headers
smtk_public_headers(smtkPQWidgetsExt ${PQWidgetsHeaders})

qt5_wrap_cpp(MOC_BUILT_SOURCES ${PQWidgetsMocHeaders})
qt5_wrap_ui(UI_BUILT_SOURCES ${PQWidgetsUIs})
qt5_add_resources(RCS_RESOURCES qtWidgetsIcons.qrc)

source_group("Qt_Source" FILES ${PQWidgetsSrcs} ${PQWidgetsUIs})
source_group("Qt_Generated" FILES ${MOC_BUILT_SOURCES} ${UI_BUILT_SOURCES} ${RCS_RESOURCES})
source_group("Qt_Header" FILES ${PQWidgetsMocHeaders})

add_library(smtkPQWidgetsExt
  ${PQWidgetsSrcs}
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${RCS_RESOURCES}
)

# Need to add the location of the moc files to the include dirs:
target_include_directories(smtkPQWidgetsExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#set smtkPQWidgetsExt to publicly link to smtkCore
target_link_libraries(smtkPQWidgetsExt LINK_PUBLIC
  pqApplicationComponents
  vtkPVServerManagerRendering
  smtkCore
  smtkQtExt
  Qt5::Core
  Qt5::Widgets
)

smtk_export_header(smtkPQWidgetsExt Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPQWidgetsExt)

set(PQWidgetsPluginHdrs
  pqSMTKWidgetsAutoStart.h
)

set(PQWidgetsPluginSrcs
  pqSMTKWidgetsAutoStart.cxx
)

add_paraview_auto_start(PLUGIN_AUTOSTART_IFACES PLUGIN_AUTOSTART_IFACE_SRCS
  CLASS_NAME pqSMTKWidgetsAutoStart
  STARTUP startup
  SHUTDOWN shutdown
)

pv_qt_wrap_cpp(PLUGIN_MOC_SRCS ${PQWidgetsPluginHdrs})
add_paraview_plugin(
  smtkPQWidgetsPlugin "1.0"
  GUI_INTERFACES
    ${PLUGIN_ACTION_IFACES}
    ${PLUGIN_AUTOSTART_IFACES}
    ${PLUGIN_PROXY_IFACES}
    ${PLUGIN_DOCK_IFACES}
  GUI_SOURCES
    ${PQWidgetsPluginHdrs}
    ${PQWidgetsPluginSrcs}
    ${PLUGIN_MOC_SRCS}
    ${PLUGIN_ACTION_IFACE_SRCS}
    ${PLUGIN_AUTOSTART_IFACE_SRCS}
    ${PLUGIN_PROXY_IFACE_SRCS}
    ${PLUGIN_DOCK_IFACES_SRCS}
)
set(SMTK_PLUGINS "${SMTK_PLUGINS};smtkPQWidgetsPlugin" CACHE INTERNAL "")

target_link_libraries(smtkPQWidgetsPlugin
  LINK_PUBLIC
  smtkPQWidgetsExt
)
smtk_export_header(smtkPQWidgetsPlugin PluginExports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPQWidgetsPlugin)

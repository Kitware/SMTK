find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# smtk views that need vtk support
set(viewSrcs
  qtSimpleExpressionEvaluationView.cxx
  )
set(viewHdrs
  qtSimpleExpressionEvaluationView.h
  )
# set up sources to build
set(PQWidgetsSrcs
  pqConePropertyWidget.cxx
  pqGenerateContoursDialog.cxx
  pqPointPropertyWidget.cxx
  pqSMTKAttributeItemWidget.cxx
  pqSMTKBoxItemWidget.cxx
  pqSMTKConeItemWidget.cxx
  pqSMTKInfiniteCylinderItemWidget.cxx
  pqSMTKLineItemWidget.cxx
  pqSMTKPlaneItemWidget.cxx
  pqSMTKPointItemWidget.cxx
  pqSMTKSphereItemWidget.cxx
  pqSMTKSplineItemWidget.cxx
  qtArcWidget.cxx
  qtInteractionWidget.cxx
  ${viewSrcs}
)

set(PQWidgetsUIs
  qtArcWidget.ui
  qtGenerateContoursDialog.ui
  resources/pqConePropertyWidget.ui
  resources/pqPointPropertyWidget.ui
)

set(PQWidgetsMocHeaders
  pqConePropertyWidget.h
  pqGenerateContoursDialog.h
  pqPointPropertyWidget.h
  pqSMTKAttributeItemWidget.h
  pqSMTKBoxItemWidget.h
  pqSMTKConeItemWidget.h
  pqSMTKInfiniteCylinderItemWidget.h
  pqSMTKLineItemWidget.h
  pqSMTKPlaneItemWidget.h
  pqSMTKPointItemWidget.h
  pqSMTKSphereItemWidget.h
  pqSMTKSplineItemWidget.h
  qtArcWidget.h
  qtInteractionWidget.h
  ${viewHdrs}
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
  vtkSMTKWidgetsExt
  smtkCore
  smtkQtExt
  Qt5::Core
  Qt5::Widgets
)

smtk_export_header(smtkPQWidgetsExt Exports.h)

set(viewInterfaces)
set(viewSources)
foreach(viewSrc ${viewSrcs})
  string(REPLACE ".cxx" "" srcClass ${viewSrc})
  set(vitmp)
  set(vstmp)
  add_smtk_ui_view(
    vitmp
    vstmp
    CLASS_NAME ${srcClass}
    VIEW_NAME ${srcClass}
  )
  list(APPEND viewInterfaces ${vitmp})
  list(APPEND viewSources ${vstmp})
endforeach()

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
    ${viewInterfaces}
  GUI_SOURCES
    ${PQWidgetsPluginHdrs}
    ${PQWidgetsPluginSrcs}
    ${PLUGIN_MOC_SRCS}
    ${PLUGIN_ACTION_IFACE_SRCS}
    ${PLUGIN_AUTOSTART_IFACE_SRCS}
    ${PLUGIN_PROXY_IFACE_SRCS}
    ${PLUGIN_DOCK_IFACES_SRCS}
    ${viewSources}
  CS_KITS
    vtkSMTKWidgetsExt
)
set_property(GLOBAL APPEND PROPERTY SMTK_PLUGINS "smtkPQWidgetsPlugin")
enable_smtk_plugin_by_default(smtkPQWidgetsPlugin TRUE)

target_link_libraries(smtkPQWidgetsPlugin
  LINK_PUBLIC
  smtkPQWidgetsExt
)
smtk_export_header(smtkPQWidgetsPlugin PluginExports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPQWidgetsExt)
smtk_install_library(smtkPQWidgetsPlugin)

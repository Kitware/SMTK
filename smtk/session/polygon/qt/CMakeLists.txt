find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# set up sources to build
set(PQWidgetsSrcs
  pqArcWidgetManager.cxx
  pqArcWidgetPanel.cxx
  pqPolygonArc.cxx
  pqSplitEdgeWidget.cxx
)

set(PQWidgetsUIs
  qtArcWidgetPanel.ui
)

set(PQWidgetsMocHeaders
  pqArcWidgetManager.h
  pqArcWidgetPanel.h
  pqPolygonArc.h
  pqSplitEdgeWidget.h
)

#install the headers
smtk_public_headers(smtkPolygonQtExt ${PQWidgetsMocHeaders} vtkPolygonArcInfo.h)

set(CMAKE_AUTOUIC 1)
set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTORCC 1)

add_library(smtkPolygonQtExt
  ${PQWidgetsSrcs}
  vtkPolygonArcInfo.cxx
  ${PQWidgetsMocHeaders}
  ${PQWidgetsUIs}
  qtWidgetsIcons.qrc
)

# Need to add the location of the moc files to the include dirs:
target_include_directories(smtkPolygonQtExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#set smtkPolygonQtExt to publicly link to smtkCore
target_link_libraries(smtkPolygonQtExt LINK_PUBLIC
  smtkCore
  smtkQtExt
  vtkPolygonOperationsExt
  vtkSMTKSourceExt
  vtkSMTKFilterExt
  ParaView::ClientServerCoreCore
  ParaView::pqApplicationComponents
  Qt5::Core
  Qt5::Widgets
  VTK::CommonCore
  VTK::DomainsChemistryOpenGL2
LINK_PRIVATE
  ParaView::ClientServer
  ParaView::ClientServerCoreRendering
  ParaView::ServerManagerCore
  ParaView::ServerManagerDefault
  ParaView::ServerManagerRendering
  ParaView::VTKExtensionsCore
  VTK::CommonDataModel
  VTK::CommonExecutionModel
  VTK::InteractionWidgets
  VTK::RenderingCore
)

smtk_export_header(smtkPolygonQtExt Exports.h)

#install the library and exports the library when used from a build tree
set(PROJECT_NAME SMTKPolygonModules)
smtk_install_library(smtkPolygonQtExt)

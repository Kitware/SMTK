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

qt5_wrap_cpp(MOC_BUILT_SOURCES ${PQWidgetsMocHeaders})
qt5_wrap_ui(UI_BUILT_SOURCES ${PQWidgetsUIs})
qt5_add_resources(RCS_RESOURCES qtWidgetsIcons.qrc)

source_group("Qt_Source" FILES ${PQWidgetsSrcs} ${PQWidgetsUIs})
source_group("Qt_Generated" FILES ${MOC_BUILT_SOURCES} ${UI_BUILT_SOURCES} ${RCS_RESOURCES})
source_group("Qt_Header" FILES ${PQWidgetsMocHeaders})

add_library(smtkPolygonQtExt
  ${PQWidgetsSrcs}
  vtkPolygonArcInfo.cxx
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${RCS_RESOURCES}
)

# Need to add the location of the moc files to the include dirs:
target_include_directories(smtkPolygonQtExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#set smtkPolygonQtExt to publicly link to smtkCore
target_link_libraries(smtkPolygonQtExt LINK_PUBLIC
  pqApplicationComponents
  smtkCore
  smtkPQWidgetsExt
  smtkQtExt
  vtkPolygonOperationsExt
  vtkSMTKSourceExt
  vtkDomainsChemistryOpenGL2
  vtkSMTKFilterExt
  Qt5::Core
  Qt5::Widgets
LINK_PRIVATE
  ${VTK_LIBRARIES}
)

smtk_export_header(smtkPolygonQtExt Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPolygonQtExt)

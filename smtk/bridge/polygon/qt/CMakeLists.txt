include(${QT_USE_FILE})

# set up sources to build
set(PQWidgetsSrcs
  pqArcWidgetManager.cxx
  pqArcWidgetPanel.cxx
  pqPolygonArc.cxx
)

set(PQWidgetsUIs
  qtArcWidgetPanel.ui
)

set(PQWidgetsMocHeaders
  pqArcWidgetManager.h
  pqArcWidgetPanel.h
  pqPolygonArc.h
)

#install the headers
smtk_public_headers(${PQWidgetsMocHeaders})

qt4_wrap_cpp(MOC_BUILT_SOURCES ${PQWidgetsMocHeaders})
qt4_wrap_ui(UI_BUILT_SOURCES ${PQWidgetsUIs})
qt4_add_resources(RCS_RESOURCES qtWidgetsIcons.qrc)

source_group("Qt_Source" FILES ${PQWidgetsSrcs} ${PQWidgetsUIs})
source_group("Qt_Generated" FILES ${MOC_BUILT_SOURCES} ${UI_BUILT_SOURCES} ${RCS_RESOURCES})
source_group("Qt_Header" FILES ${PQWidgetsMocHeaders})

add_library(smtkPolygonQtExt
  ${PQWidgetsSrcs}
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${RCS_RESOURCES}
)

#we need to add the location of the moc files to the include dir for smtkPWWidgetExt
target_include_directories(smtkPolygonQtExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
#qt4_use_modules(smtkPolygonQtExt LINK_PUBLIC Core Gui)

#set smtkPolygonQtExt to publicly link to smtkCore
target_link_libraries(smtkPolygonQtExt LINK_PUBLIC
  pqApplicationComponents
  smtkCore
  smtkPQWidgetsExt
  vtkPolygonOperatorsExt
)

smtk_export_header(smtkPolygonQtExt Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPolygonQtExt)

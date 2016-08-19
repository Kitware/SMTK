include(${QT_USE_FILE})

# set up sources to build
set(PQWidgetsSrcs
  pq3DWidget.cxx
  pq3DWidgetInterface.cxx
  pqArcWidget.cxx
  pqLineWidget.cxx
  pqGenerateContoursDialog.cxx
)

set(PQWidgetsUIs
  qtArcWidget.ui
  qtLineWidget.ui
  qtGenerateContoursDialog.ui
)

set(PQWidgetsMocHeaders
  pq3DWidget.h
  pqArcWidget.h
  pqLineWidget.h
  pqGenerateContoursDialog.h
)

set(PQWidgetsHeaders
  ${PQWidgetsMocHeaders}
  pq3DWidgetInterface.h
)

#install the headers
smtk_public_headers(${PQWidgetsHeaders})

qt4_wrap_cpp(MOC_BUILT_SOURCES ${PQWidgetsMocHeaders})
qt4_wrap_ui(UI_BUILT_SOURCES ${PQWidgetsUIs})
qt4_add_resources(RCS_RESOURCES qtWidgetsIcons.qrc)

source_group("Qt_Source" FILES ${PQWidgetsSrcs} ${PQWidgetsUIs})
source_group("Qt_Generated" FILES ${MOC_BUILT_SOURCES} ${UI_BUILT_SOURCES} ${RCS_RESOURCES})
source_group("Qt_Header" FILES ${PQWidgetsMocHeaders})

add_library(smtkPQWidgetsExt
  ${PQWidgetsSrcs}
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${RCS_RESOURCES}
)

#we need to add the location of the moc files to the include dir for smtkPWWidgetExt
target_include_directories(smtkPQWidgetsExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
#qt4_use_modules(smtkPQWidgetsExt LINK_PUBLIC Core Gui)

#set smtkPQWidgetsExt to publicly link to smtkCore
target_link_libraries(smtkPQWidgetsExt LINK_PUBLIC
  pqApplicationComponents
  vtkPVServerManagerRendering
  smtkCore
  ${QT_QTGUI_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  )

if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_extension_paraview_widgets LINK_PUBLIC
    smtkPQWidgetsExt
    )
endif()

smtk_export_header(smtkPQWidgetsExt Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPQWidgetsExt)

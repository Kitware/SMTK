smtk_find_package_qt(qt_targets REQUIRED
  QT4_COMPONENTS QtCore QtGui
  QT5_COMPONENTS Core Widgets)

# set up sources to build
set(PQWidgetsSrcs
  pqGenerateContoursDialog.cxx
  qtArcWidget.cxx
  qtInteractionWidget.cxx
  qtLineWidget.cxx
)

set(PQWidgetsUIs
  qtArcWidget.ui
  qtLineWidget.ui
  qtGenerateContoursDialog.ui
)

set(PQWidgetsMocHeaders
  pqGenerateContoursDialog.h
  qtArcWidget.h
  qtInteractionWidget.h
  qtLineWidget.h
)

set(PQWidgetsHeaders
  ${PQWidgetsMocHeaders}
)

#install the headers
smtk_public_headers(${PQWidgetsHeaders})

smtk_qt_wrap_cpp(MOC_BUILT_SOURCES ${PQWidgetsMocHeaders})
smtk_qt_wrap_ui(UI_BUILT_SOURCES ${PQWidgetsUIs})
smtk_qt_add_resources(RCS_RESOURCES qtWidgetsIcons.qrc)

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

#set smtkPQWidgetsExt to publicly link to smtkCore
target_link_libraries(smtkPQWidgetsExt LINK_PUBLIC
  pqApplicationComponents
  vtkPVServerManagerRendering
  smtkCore
  ${qt_targets}
  )

if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_extension_paraview_widgets LINK_PUBLIC
    smtkPQWidgetsExt
    )
endif()

smtk_export_header(smtkPQWidgetsExt Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPQWidgetsExt)

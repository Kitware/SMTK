find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

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
  Qt5::Core
  Qt5::Widgets
  )

smtk_export_header(smtkPQWidgetsExt Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPQWidgetsExt)

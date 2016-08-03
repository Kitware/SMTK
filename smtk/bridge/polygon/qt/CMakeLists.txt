include(${QT_USE_FILE})

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
smtk_public_headers(${PQWidgetsMocHeaders} vtkPolygonArcInfo.h)

qt4_wrap_cpp(MOC_BUILT_SOURCES ${PQWidgetsMocHeaders})
qt4_wrap_ui(UI_BUILT_SOURCES ${PQWidgetsUIs})
qt4_add_resources(RCS_RESOURCES qtWidgetsIcons.qrc)

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

#we need to add the location of the moc files to the include dir for smtkPWWidgetExt
target_include_directories(smtkPolygonQtExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
#qt4_use_modules(smtkPolygonQtExt LINK_PUBLIC Core Gui)

#set smtkPolygonQtExt to publicly link to smtkCore
target_link_libraries(smtkPolygonQtExt LINK_PUBLIC
  pqApplicationComponents
  smtkCore
  smtkPQWidgetsExt
  vtkPolygonOperatorsExt
  vtkSMTKSourceExt
  vtkDomainsChemistryOpenGL2
)

smtk_export_header(smtkPolygonQtExt Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPolygonQtExt)

if (SMTK_ENABLE_TESTING)
  # Link *everything* in this directory to VTK modules
  # This is required for the TestBuild targets.
  # We must test whether each of the targets below exist
  # because they are required when built into VTK but
  # may not be present, especially since the OpenGL2
  # backend became available.
  set(SMTK_VTK_TARGET_LIST)
  foreach(target
    vtkInteractionStyle
    vtkRenderingFreeType
    vtkRendering${VTK_RENDERING_BACKEND}
    )
    if (TARGET ${target})
      list(APPEND SMTK_VTK_TARGET_LIST ${target})
    endif()
  endforeach()

  target_link_libraries(TestBuild_smtk_bridge_polygon_qt PUBLIC ${SMTK_VTK_TARGET_LIST})
endif()

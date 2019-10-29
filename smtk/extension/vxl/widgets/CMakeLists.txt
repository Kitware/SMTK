find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

set(VXLWidgetsHeaders
  pqTerrainExtractionManager.h
   )
set(VXLWidgetsSrcs
  pqTerrainExtractionManager.cxx
   )
set(VXLWidgetsUI
   )

#install the headers
smtk_public_headers(smtkVXLWidgetsExt ${VXLWidgetsHeaders})

set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTOUIC 1)

add_library(smtkVXLWidgetsExt
  ${VXLWidgetsSrcs}
  ${VXLWidgetsHeaders}
  ${VXLWidgetsUI}
)

target_include_directories(smtkVXLWidgetsExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(smtkVXLWidgetsExt LINK_PUBLIC
  smtkCore
  vtkSMTKSourceExt
  ParaView::pqApplicationComponents
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
LINK_PRIVATE
  ${VTK_LIBRARIES}
  smtkQtExt
)

smtk_export_header(smtkVXLWidgetsExt Exports.h)

#install the library and exports the library when used from a build tree
set(PROJECT_NAME SMTKVXLExt)
smtk_install_library(smtkVXLWidgetsExt)

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
    vtkRenderingOpenGL2
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    )
    if (TARGET ${target})
      list(APPEND SMTK_VTK_TARGET_LIST ${target})
    endif()
  endforeach()
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
#include(${PARAVIEW_USE_FILE})
#include(ParaViewPlugins)

# set up sources to build
set(srcs
  Registrar.cxx
  smtkTerrainExtractionView.cxx
)

set(hdrs
  Registrar.h
  smtkTerrainExtractionView.h
)

set(qtui
  smtkTerrainExtractionParameters.ui
)

#install the headers
smtk_public_headers(smtkVXLOperationViewsExt ${hdrs})

set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTOUIC 1)

add_library(smtkVXLOperationViewsExt
  ${srcs}
  ${hdrs}
  ${qtui}
)

#set smtkVXLOperationViewsExt to publicly link to smtkCore
target_link_libraries(smtkVXLOperationViewsExt
  LINK_PUBLIC
    smtkQtExt
    smtkCore
    ParaView::pqApplicationComponents
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
  LINK_PRIVATE
    smtkVXLWidgetsExt
)

smtk_export_header(smtkVXLOperationViewsExt Exports.h)

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  set_property(GLOBAL APPEND
    PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
endif()

#install the library and exports the library when used from a build tree
smtk_install_library(smtkVXLOperationViewsExt)

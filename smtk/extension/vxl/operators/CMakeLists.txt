find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
#include(${PARAVIEW_USE_FILE})
#include(ParaViewPlugins)

# set up sources to build
set(srcs
  smtkTerrainExtractionView.cxx
)

set(hdrs
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
    pqApplicationComponents
    smtkCore
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
  LINK_PRIVATE
    smtkVXLWidgetsExt
    smtkPQWidgetsExt
)

smtk_export_header(smtkVXLOperationViewsExt Exports.h)

unset(CMAKE_AUTOMOC)

# Generate the ParaView plugin code for each view class
# added to the smtkVXLOperationViewsExt library above:
set(viewInterfaces)
set(viewSources)
foreach(src ${srcs})
  string(REPLACE ".cxx" "" srcClass ${src})
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

add_paraview_plugin(
  smtkVXLOperationViewsPlugin "1.0"
  #SERVER_MANAGER_XML smconfig.xml
  GUI_INTERFACES ${viewInterfaces}
  GUI_SOURCES ${viewSources}
)
target_link_libraries(smtkVXLOperationViewsPlugin
  LINK_PUBLIC
    smtkVXLOperationViewsExt
)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkVXLOperationViewsExt)
smtk_install_library(smtkVXLOperationViewsPlugin)

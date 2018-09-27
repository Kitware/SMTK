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

qt5_wrap_cpp(MOC_BUILT_SOURCES ${hdrs})
qt5_wrap_ui(UI_BUILT_SOURCES ${qtui})

source_group("Qt_Source" FILES ${srcs} ${qtui} )
source_group("Qt_Generated" FILES ${MOC_BUILT_SOURCES} ${UI_BUILT_SOURCES})
source_group("Qt_Header" FILES ${hdrs})

add_library(smtkVXLOperationViewsExt
  ${srcs}
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
)

#we need to add the location of the moc files to the include dir for smtkVXLWidgetExt
target_include_directories(smtkVXLOperationViewsExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

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
  CS_KITS
    vtkSMTKWidgetsExt
)
target_link_libraries(smtkVXLOperationViewsPlugin
  LINK_PUBLIC
    smtkVXLOperationViewsExt
)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkVXLOperationViewsExt)
smtk_install_library(smtkVXLOperationViewsPlugin)

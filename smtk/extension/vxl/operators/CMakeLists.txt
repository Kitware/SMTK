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
smtk_public_headers(${hdrs})

if (PARAVIEW_QT_VERSION VERSION_GREATER "4")
  qt5_wrap_cpp(mocs ${hdrs})
  qt5_wrap_ui(uisrc ${qtui})
else()
  qt4_wrap_cpp(mocs ${hdrs})
  qt4_wrap_ui(uisrc ${qtui})
endif()

source_group("Qt_Source" FILES ${srcs} )
source_group("Qt_Generated" FILES ${mocs} ${uisrc})
source_group("Qt_Header" FILES ${hdrs})

add_library(smtkVXLOperatorViewsExt
  ${srcs}
  ${mocs}
  ${uisrc}
)

#we need to add the location of the moc files to the include dir for smtkVXLWidgetExt
target_include_directories(smtkVXLOperatorViewsExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#set smtkVXLOperatorViewsExt to publicly link to smtkCore
target_link_libraries(smtkVXLOperatorViewsExt
  LINK_PUBLIC
    smtkQtExt
    pqApplicationComponents
    smtkCore
  LINK_PRIVATE
    smtkPQWidgetsExt
    #${QT_QTCORE_LIBRARY} # it seems that it's deprecated
    ${Boost_LIBRARIES}
    vtkjsoncpp
)

# if (SMTK_ENABLE_TESTING)
#   target_link_libraries(TestBuild_smtk_extension_paraview_operators
#     LINK_PUBLIC
#     smtkVXLOperatorViewsExt
#     )
# endif()

smtk_export_header(smtkVXLOperatorViewsExt Exports.h)

# Generate the ParaView plugin code for each view class
# added to the smtkVXLOperatorViewsExt library above:
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
  smtkVXLOperatorViewsPlugin "1.0"
  #SERVER_MANAGER_XML smconfig.xml
  #SERVER_SOURCES sessionInit.cxx # do I need it here?
  GUI_INTERFACES ${viewInterfaces}
  GUI_SOURCES ${viewSources}
  CS_KITS
    vtkSMTKWidgetsExt
    # TBDadd other libraries in vxl
)
target_link_libraries(smtkVXLOperatorViewsPlugin
  LINK_PUBLIC
    smtkVXLOperatorViewsExt
)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkVXLOperatorViewsExt)
smtk_install_library(smtkVXLOperatorViewsPlugin)

include(${PARAVIEW_USE_FILE})
include(ParaViewPlugins)

# set up sources to build
set(srcs
  smtkAssignColorsView.cxx
  smtkSaveModelView.cxx
  smtkExportModelView.cxx
)

set(hdrs
  smtkAssignColorsView.h
  smtkModelIOView.h
  smtkSaveModelView.h
  smtkExportModelView.h
)

set(qtui
  smtkAssignColorsParameters.ui
  smtkSaveModelParameters.ui
  smtkExportModelParameters.ui
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

add_library(smtkPQOperatorViewsExt
  ${srcs}
  ${mocs}
  ${uisrc}
)

#we need to add the location of the moc files to the include dir for smtkPWWidgetExt
target_include_directories(smtkPQOperatorViewsExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
#qt4_use_modules(smtkPQOperatorViewsExt LINK_PUBLIC Core Gui)

#set smtkPQOperatorViewsExt to publicly link to smtkCore
target_link_libraries(smtkPQOperatorViewsExt
  LINK_PUBLIC
    pqApplicationComponents
    smtkQtExt
    smtkCore
    ${QT_QTCORE_LIBRARY}
    ${Boost_LIBRARIES}
    vtkjsoncpp
)

if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_extension_paraview_operators
    LINK_PUBLIC
    smtkPQOperatorViewsExt
    )
endif()

smtk_export_header(smtkPQOperatorViewsExt Exports.h)

# Generate the ParaView plugin code for each view class
# added to the smtkPQOperatorViewsExt library above:
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
  smtkPQOperatorViewsPlugin "1.0"
  GUI_INTERFACES ${viewInterfaces}
  GUI_SOURCES ${viewSources}
  CS_KITS
  vtkSMTKWidgetsExt
)
target_link_libraries(smtkPQOperatorViewsPlugin
  LINK_PUBLIC
    smtkPQOperatorViewsExt
)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPQOperatorViewsExt)
smtk_install_library(smtkPQOperatorViewsPlugin)

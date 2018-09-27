find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

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

set(unwrappedHdrs
  PrepareToSave.h
)

set(qtui
  smtkAssignColorsParameters.ui
  smtkSaveModelParameters.ui
  smtkExportModelParameters.ui
)

#install the headers
smtk_public_headers(smtkPQOperationViewsExt ${hdrs} ${unwrappedHdrs})

qt5_wrap_cpp(mocs ${hdrs})
qt5_wrap_ui(uisrc ${qtui})

source_group("Qt_Source" FILES ${srcs} )
source_group("Qt_Generated" FILES ${mocs} ${uisrc})
source_group("Qt_Header" FILES ${hdrs})

add_library(smtkPQOperationViewsExt
  ${srcs}
  ${mocs}
  ${uisrc}
)

#we need to add the location of the moc files to the include dir for smtkPQWidgetExt
target_include_directories(smtkPQOperationViewsExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#set smtkPQOperationViewsExt to publicly link to smtkCore
target_link_libraries(smtkPQOperationViewsExt
  LINK_PUBLIC
    pqApplicationComponents
    smtkQtExt
    smtkCore
    Qt5::Core
    Qt5::Widgets
    ${Boost_LIBRARIES}
    vtkjsoncpp
)

smtk_export_header(smtkPQOperationViewsExt Exports.h)

# Generate the ParaView plugin code for each view class
# added to the smtkPQOperationViewsExt library above:
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
  smtkPQOperationViewsPlugin "1.0"
  GUI_INTERFACES ${viewInterfaces}
  GUI_SOURCES ${viewSources}
  CS_KITS
  vtkSMTKWidgetsExt
)
set(SMTK_PLUGINS "${SMTK_PLUGINS};smtkPQOperationViewsPlugin" CACHE INTERNAL "")
target_link_libraries(smtkPQOperationViewsPlugin
  LINK_PUBLIC
    smtkPQOperationViewsExt
)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPQOperationViewsExt)
smtk_install_library(smtkPQOperationViewsPlugin)

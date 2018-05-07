find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

include(${PARAVIEW_USE_FILE})
include (ParaViewPlugins)

# We need to add the current value of VTK_MODULES_DIR to the module path
# so that when the plugins are built all the modules can be found. Otherwise,
# modules that aren't loaded as direct dependencies of CMB modules will
# not be found.
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

set (srcs
  smtkRGGAddMaterialView.cxx
  smtkRGGEditAssemblyView.cxx
  smtkRGGEditCoreView.cxx
  smtkRGGEditDuctView.cxx
  smtkRGGEditMaterialView.cxx
  smtkRGGEditPinView.cxx
  smtkRGGRemoveMaterialView.cxx
  smtkRGGReadRXFFileView.cxx
)

set (non_ui_srcs
  smtkRGGViewHelper.cxx
  rggAuxiliaryGeometryExtension.cxx
)

set (hdrs
  smtkRGGAddMaterialView.h
  smtkRGGEditAssemblyView.h
  smtkRGGEditCoreView.h
  smtkRGGEditDuctView.h
  smtkRGGEditMaterialView.h
  smtkRGGEditPinView.h
  smtkRGGReadRXFFileView.h
  smtkRGGRemoveMaterialView.h
)

set (qtui
  smtkRGGAddMaterialParameters.ui
  smtkRGGEditAssemblyParameters.ui
  smtkRGGEditCoreParameters.ui
  smtkRGGEditDuctParameters.ui
  smtkRGGEditMaterialParameters.ui
  smtkRGGEditPinParameters.ui
  smtkRGGReadRXFFileParameters.ui
  smtkRGGRemoveMaterialParameters.ui
)

#install the headers
smtk_public_headers(${hdrs})

qt5_wrap_cpp(mocs ${hdrs})
qt5_wrap_ui(uisrc ${qtui})

source_group("Qt_Source" FILES ${srcs} )
source_group("Qt_Generated" FILES ${mocs} ${uisrc})
source_group("Qt_Header" FILES ${hdrs})

# We must add the custom view in the plugin dir instead of qt dir
# due to the fact that add_smtk__ui_view marcro assumes files
# are in the same dir.
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
  smtkRGGSessionPlugin "1.0"
  GUI_INTERFACES ${viewInterfaces}
  GUI_SOURCES
    ${viewSources}
    ${srcs}
    ${mocs}
    ${uisrc}
  SOURCES
    ${non_ui_srcs}
  SERVER_SOURCES
    sessionInit.cxx
  CS_KITS
    vtkSMTKWidgetsExt
)

target_link_libraries(smtkRGGSessionPlugin
  LINK_PUBLIC
    smtkCore
    smtkRGGSession
    smtkQtRGGSession
    smtkQtRGGNuclides
    Qt5::Core
    Qt5::Widgets
    vtkjsoncpp
    vtkPVServerManagerApplication
  LINK_PRIVATE
    vtkPVServerManagerApplicationCS
    vtkSMTKWidgetsExtCS
)
if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_bridge_rgg_plugin
    LINK_PUBLIC
    smtkRGGSessionPlugin
    )
endif()

smtk_export_header(smtkRGGSessionPlugin Exports.h)
smtk_install_library(smtkRGGSessionPlugin)

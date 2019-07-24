find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

include(${PARAVIEW_USE_FILE})
include (ParaViewPlugins)

# We need to add the current value of VTK_MODULES_DIR to the module path
# so that when the plugins are built all the modules can be found. Otherwise,
# modules that aren't loaded as direct dependencies of CMB modules will
# not be found.
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

set (srcs
)

set (non_ui_srcs
  oscillatorAuxiliaryGeometryExtension.cxx
)

set (hdrs
  oscillatorAuxiliaryGeometryExtension.h
)

set (qtui
)

# Install the headers
smtk_public_headers(smtkOscillatorSessionPlugin ${hdrs})

# We must add custom views in the plugin dir instead of qt dir
# due to the fact that the add_smtk_ui_view macro assumes files
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

add_smtk_plugin(
  smtkOscillatorSessionPlugin "1.0"
  GUI_INTERFACES ${viewInterfaces}
  GUI_SOURCES
    ${viewSources}
    ${srcs}
    ${mocs}
    ${uisrc}
  SOURCES
    ${non_ui_srcs}
  REGISTRAR
    smtk::session::oscillator::Registrar
  MANAGERS
    smtk::operation::Manager smtk::resource::Manager
)

target_link_libraries(smtkOscillatorSessionPlugin
  LINK_PUBLIC
    smtkCore
    smtkOscillatorSession
    # smtkQtOscillatorSession
    # Qt5::Core
    # Qt5::Widgets
    # vtkjsoncpp
    # vtkPVServerManagerApplication
  LINK_PRIVATE
    vtkPVServerManagerApplicationCS
    vtkSMTKWidgetsExtCS
    vtkSMTKSourceExt
)

smtk_export_header(smtkOscillatorSessionPlugin Exports.h)
smtk_install_library(smtkOscillatorSessionPlugin)

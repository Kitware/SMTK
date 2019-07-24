find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Widgets)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/girderfilebrowser")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/girderfilebrowser/ui")

set(ui_files
  ui/jobtablewidget.ui
  ui/mainwindow.ui
  ui/logindialog.ui
  ui/cumuluswidget.ui
  # Mandatory girder file browser files
  girderfilebrowser/ui/girderfilebrowserdialog.ui
  # Optional girder file browser files (for stand-alone executable)
  girderfilebrowser/ui/girderlogindialog.ui
)

set(src
  jobtablewidget.cxx
  jobview.cxx
  logindialog.cxx
  jobtablemodel.cxx
  job.cxx
  cumulusproxy.cxx
  girderrequest.cxx
  jobrequest.cxx
  utils.cxx
  cumuluswidget.cxx
  mainwindow.cxx
  pqCumulusJobsPanel.cxx
  # Mandatory girder file browser files
  girderfilebrowser/girderfilebrowserfetcher.cxx
  girderfilebrowser/ui/girderfilebrowserdialog.cxx
  girderfilebrowser/ui/girderfilebrowserlistview.cxx
  # Optional girder file browser files (for stand-alone executable)
  girderfilebrowser/girderauthenticator.cxx
  girderfilebrowser/ui/girderlogindialog.cxx
)

#add moc files
set(mochdrs
  cumulusproxy.h
  girderrequest.h
  jobrequest.h
  jobtablemodel.h
  jobtablewidget.h
  jobview.h
  mainwindow.h
  pqCumulusJobsPanel.h
  # Mandatory girder file browser files
  girderfilebrowser/girderfilebrowserfetcher.h
  girderfilebrowser/ui/girderfilebrowserdialog.h
  girderfilebrowser/ui/girderfilebrowserlistview.h
  # Optional girder file browser files (for stand-alone executable)
  girderfilebrowser/girderauthenticator.h
  girderfilebrowser/ui/girderlogindialog.h
)

#resource files
set(resources
 girderfilebrowser/resources.qrc
)

set(CMAKE_AUTOUIC 1)
set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTORCC 1)

#install headers
set(hdrs
  cumuluswidget.h
  job.h
  logindialog.h
)
#extension library
add_library(smtkCumulusExt
 ${src}
 ${hdrs}
 ${mochdrs}
 ${ui_files}
 ${resources}
)
smtk_public_headers(smtkCumulusExt ${hdrs})

#we need to add the location of the moc files to the include dir
target_include_directories(smtkCumulusExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#publicly link to smtkCore
target_link_libraries(smtkCumulusExt LINK_PUBLIC
  smtkCore
  Qt5::Core
  Qt5::Gui
  Qt5::Network
  Qt5::Widgets
)
smtk_export_header(smtkCumulusExt Exports.h)

#install the library and exports
smtk_install_library(smtkCumulusExt)

#standalone cumulus executable
add_executable(cumulus MACOSX_BUNDLE
  main.cxx
)
#standalone girder file browser executable for debugging
add_executable(girderfilebrowser MACOSX_BUNDLE WIN32
  girderfilebrowser/girderfilebrowser.cxx
)

set_target_properties(cumulus PROPERTIES AUTOMOC TRUE)
set_target_properties(girderfilebrowser PROPERTIES AUTOMOC TRUE)

target_link_libraries(cumulus
  smtkCumulusExt
  cJSON
)
target_link_libraries(girderfilebrowser
  smtkCumulusExt
)

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_paraview_dock_window(cumulusJobsPanelIfaces cumulusJobsPanelIfaceSrcs
    CLASS_NAME pqCumulusJobsPanel
    )
  list(APPEND PLUGIN_DOCK_IFACES ${cumulusJobsPanelIfaces})
  list(APPEND PLUGIN_DOCK_IFACES_SRCS ${cumulusJobsPanelIfaceSrcs})

  unset(CMAKE_AUTOMOC)

  add_paraview_plugin(
    smtkPQCumulusJobsPlugin "1.0"
    GUI_INTERFACES
    ${PLUGIN_ACTION_IFACES}
    ${PLUGIN_AUTOSTART_IFACES}
    ${PLUGIN_PROXY_IFACES}
    ${PLUGIN_DOCK_IFACES}
    GUI_SOURCES
    ${Headers}
    ${Sources}
    ${PLUGIN_MOC_SRCS}
    ${PLUGIN_ACTION_IFACE_SRCS}
    ${PLUGIN_AUTOSTART_IFACE_SRCS}
    ${PLUGIN_PROXY_IFACE_SRCS}
    ${PLUGIN_DOCK_IFACES_SRCS}
    )
  set_property(GLOBAL APPEND PROPERTY SMTK_PLUGINS "smtkPQCumulusJobsPlugin")

  target_link_libraries(smtkPQCumulusJobsPlugin
    LINK_PUBLIC
    smtkCumulusExt
    )

  smtk_install_library(smtkPQCumulusJobsPlugin)
endif()

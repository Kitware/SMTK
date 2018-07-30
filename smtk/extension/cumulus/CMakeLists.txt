find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Widgets)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(ui_files
  ui/jobtablewidget.ui
  ui/mainwindow.ui
  ui/logindialog.ui
  ui/cumuluswidget.ui
)

qt5_wrap_ui(ui_srcs ${ui_files})

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
)

#add moc files
set(mochdrs
  cumulusproxy.h
  cumuluswidget.h
  girderrequest.h
  jobrequest.h
  jobtablemodel.h
  jobtablewidget.h
  jobview.h
  logindialog.h
  mainwindow.h
  pqCumulusJobsPanel.h
)
qt5_wrap_cpp(moc_srcs ${mochdrs})

#extension library
add_library(smtkCumulusExt
 ${src}
 ${moc_srcs}
 ${ui_srcs}
)
#install headers
set(hdrs
  cumuluswidget.h
  job.h
  logindialog.h
)
smtk_public_headers(${hdrs})

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

set_target_properties(cumulus PROPERTIES AUTOMOC TRUE)

target_link_libraries(cumulus
  smtkCumulusExt
  cJSON
)

if(SMTK_ENABLE_PARAVIEW_SUPPORT)
  add_paraview_dock_window(cumulusJobsPanelIfaces cumulusJobsPanelIfaceSrcs
    CLASS_NAME pqCumulusJobsPanel
    )
  list(APPEND PLUGIN_DOCK_IFACES ${cumulusJobsPanelIfaces})
  list(APPEND PLUGIN_DOCK_IFACES_SRCS ${cumulusJobsPanelIfaceSrcs})

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
  set(SMTK_PLUGINS "${SMTK_PLUGINS};smtkPQCumulusJobsPlugin" CACHE INTERNAL "")

  target_link_libraries(smtkPQCumulusJobsPlugin
    LINK_PUBLIC
    smtkCumulusExt
    )

  smtk_install_library(smtkPQCumulusJobsPlugin)
endif()

if (SMTK_ENABLE_TESTING)
  # Make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_extension_cumulus LINK_PRIVATE smtkCumulusExt)
endif()

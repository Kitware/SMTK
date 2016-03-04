include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(ui_files
  ui/jobtablewidget.ui
  ui/mainwindow.ui
  ui/logindialog.ui
  ui/cumuluswidget.ui
)

set(qt_components
  Core
  Gui
  Network
)

qt4_wrap_ui(ui_srcs ${ui_files})

set(src
  jobtablewidget.cxx
  jobview.cxx
  mainwindow.cxx
  main.cxx
  logindialog.cxx
  jobtablemodel.cxx
  job.cxx
  cumulusproxy.cxx
  girderrequest.cxx
  jobrequest.cxx
  utils.cxx
  cumuluswidget.cxx
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
)
qt4_wrap_cpp(moc_srcs ${mochdrs})

#extension library
add_library(smtkCumulusExt
 ${src}
 ${moc_srcs}
 ${ui_srcs}
)
smtk_install_library(smtkCumulusExt)

#install headers
set(hdrs
  cumuluswidget.h
  logindialog.h
)
smtk_public_headers(${hdrs})

#standalone cumulus executable
add_executable(cumulus MACOSX_BUNDLE
    ${src}
    ${ui_srcs}
)

set_target_properties(cumulus PROPERTIES AUTOMOC TRUE)

qt4_use_modules(cumulus ${qt_components})

target_link_libraries(cumulus cJSON)

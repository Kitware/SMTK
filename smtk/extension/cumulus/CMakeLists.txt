
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(ui_files
  ui/jobtablewidget.ui
  ui/mainwindow.ui
  ui/logindialog.ui
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
)

#add in the attribute preview executable
add_executable(cumulus MACOSX_BUNDLE
    ${src}
    ${ui_srcs}
)

set_target_properties(cumulus PROPERTIES AUTOMOC TRUE)

qt4_use_modules(cumulus ${qt_components})

target_link_libraries(cumulus cJSON)

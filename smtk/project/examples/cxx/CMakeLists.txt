if(SMTK_ENABLE_QT_SUPPORT)
  ##
  ## Create browseModel to test browsing model entity tree:
  find_package(Qt5 COMPONENTS Widgets)

  set(CMAKE_AUTOMOC 1)
  set(CMAKE_AUTOUIC 1)

  add_executable(browseProject
    browseProject.cxx
    ProjectBrowser.cxx
    ProjectBrowser.ui)
  target_link_libraries(browseProject
    smtkCore
    smtkQtExt
  )

  set_target_properties(
    browseProject PROPERTIES
    INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/bin")

endif()

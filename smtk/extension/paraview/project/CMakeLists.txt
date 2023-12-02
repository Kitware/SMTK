set(classes
  pqSMTKDisplayProjectOnLoadBehavior
  pqSMTKProjectAutoStart
  pqSMTKProjectBrowser
  pqSMTKProjectMenu
  pqSMTKProjectPanel
  Registrar
)

smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/ProjectPanelConfiguration.json"
  TYPE "_json"
  HEADER_OUTPUT rpJsonHeader)

set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTOUIC 1)

set(headers
  ${rpJsonHeader}
  pqSMTKProjectDock.h
)

vtk_module_add_module(smtkPQProjectExt
  CLASSES ${classes}
  HEADERS ${headers}
  HEADERS_SUBDIR "smtk/extension/paraview/project"
)
vtk_module_link(smtkPQProjectExt
  PUBLIC
    smtkCore
    smtkQtExt
    Qt5::Widgets
)
vtk_module_definitions(smtkPQProjectExt PRIVATE QT_NO_KEYWORDS)

target_link_libraries(smtkPQProjectExt
  PUBLIC
    ParaView::pqApplicationComponents
    smtkCore
    smtkPQComponentsExt
    smtkPVServerExt
)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()

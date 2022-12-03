find_package(Qt5 REQUIRED COMPONENTS Core Svg Widgets)

set(headers)
set(classes
  Registrar
  qtOntologyItem
  qtOntologyModel
)
set(sources
  qtOntologyItem.qrc
)

set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTOUIC 1)
set(CMAKE_AUTORCC 1)

vtk_module_add_module(smtkPQMarkupExt
  CLASSES ${classes}
  SOURCES ${sources}
  HEADERS ${headers}
  HEADERS_SUBDIR "smtk/extension/paraview/markup")

vtk_module_link(smtkPQMarkupExt
  PUBLIC
    smtkCore
    smtkMarkup
    smtkQtExt
    nlohmann_json
    vtkSMTKMarkupExt
    Qt5::Core
    Qt5::Svg
    Qt5::Widgets
)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif ()

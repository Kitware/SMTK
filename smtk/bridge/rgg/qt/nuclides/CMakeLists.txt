# ==============================================================================
# Source
# ==============================================================================
find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Qt UI files
# ------------
set(UI_FORMS
)

qt5_wrap_ui(UIC_SOURCE  ${UI_FORMS})

# Source files
# ------------
set(SOURCE
  NuclideTable.cxx
  NuclideTable.h
  Nuclide.cxx
  Nuclide.h
  NuclideTableView.cxx
  NuclideTableView.h
  ${UIC_SOURCE})

# ==============================================================================
# Application
# ==============================================================================

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_library(smtkQtRGGNuclides ${SOURCE} NuclideResources.qrc)

set_target_properties(smtkQtRGGNuclides PROPERTIES AUTOMOC TRUE)
target_link_libraries(smtkQtRGGNuclides
  LINK_PUBLIC
  Qt5::Core
  Qt5::Widgets
  Qt5::OpenGL
)

smtk_export_header(smtkQtRGGNuclides Exports.h)
smtk_install_library(smtkQtRGGNuclides)

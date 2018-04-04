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
  main.cxx
  ${UIC_SOURCE})

# ==============================================================================
# Application
# ==============================================================================

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(Nuclides MACOSX_BUNDLE ${SOURCE})
set_target_properties(Nuclides PROPERTIES AUTOMOC TRUE
  INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/bin")
target_link_libraries(Nuclides
  LINK_PRIVATE
  smtkQtRGGNuclides
)

# ==============================================================================
# Install
# ==============================================================================

install(TARGETS Nuclides
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION Applications      # Required for MACOSX_BUNDLE
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

include(${PARAVIEW_USE_FILE})
include(ParaViewPlugins)

# We need to add the current value of VTK_MODULES_DIR to the module path
# so that when the plugins are built all the modules can be found. Otherwise,
# modules that aren't loaded as direct dependencies of CMB modules will
# not be found.
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

set (srcs
  qtDraw2DLattice.cxx
  qtDrawLatticeItem.cxx
  qtLattice.cxx
  qtLatticeHelper.cxx
)

set (non_ui_srcs
  rggLatticeContainer.cxx
  rggNucAssembly.cxx
  rggNucCore.cxx
  rggNucCoordinateConverter.cxx
)

set (moc_hdrs
  qtDraw2DLattice.h
  qtDrawLatticeItem.h
)
set (hdrs
  ${moc_hdrs}
  qtLattice.h
  qtLatticeHelper.h
  rggNucAssembly.h
  rggNucCore.h
  rggLatticeContainer.h
  rggNucCoordinateConverter.h
  rggNucPartDefinition.h
)


set (qtui
)

#install the headers
smtk_public_headers(${hdrs})

qt5_wrap_cpp(mocs ${moc_hdrs})
qt5_wrap_ui(uisrc ${qtui})

source_group("Qt_Source" FILES ${srcs} )
source_group("Qt_Generated" FILES ${mocs} ${uisrc})
source_group("Qt_Header" FILES ${hdrs})

add_library(smtkQtRGGSession
  ${srcs}
  ${non_ui_srcs}
  ${mocs}
  ${uisrc}
)

#we need to add the location of the moc files to the include dir for smtkQtRGGSession
target_include_directories(smtkQtRGGSession PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#set smtkQtRGGSession to publicly link to smtkCore
target_link_libraries(smtkQtRGGSession
  LINK_PUBLIC
    smtkQtExt
    smtkCore
    Qt5::Core
    Qt5::Widgets
LINK_PRIVATE
    ${VTK_LIBRARIES}
)

if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_bridge_rgg_qt
    LINK_PUBLIC
    smtkQtRGGSession
    )
endif()

smtk_export_header(smtkQtRGGSession Exports.h)
smtk_install_library(smtkQtRGGSession)

add_subdirectory(nuclides)

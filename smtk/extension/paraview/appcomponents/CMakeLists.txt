smtk_find_package_qt(qt_targets REQUIRED
  QT4_COMPONENTS QtCore
  QT5_COMPONENTS Core)

# set up sources to build
set(PQComponentsSrcs
  pqPluginSMTKViewBehavior.cxx
)

set(PQComponentsHeaders
  pqPluginSMTKViewBehavior.h
)

#install the headers
smtk_public_headers(${PQComponentsHeaders})

smtk_qt_wrap_cpp(MOC_BUILT_SOURCES ${PQComponentsHeaders})

source_group("Qt_Source" FILES ${PQComponentsSrcs} )
source_group("Qt_Generated" FILES ${MOC_BUILT_SOURCES} )
source_group("Qt_Header" FILES ${PQComponentsHeaders})

add_library(smtkPQComponentsExt
  ${PQComponentsSrcs}
  ${MOC_BUILT_SOURCES}
)

#we need to add the location of the moc files to the include dir for smtkPWWidgetExt
target_include_directories(smtkPQComponentsExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#set smtkPQComponentsExt to publicly link to smtkCore
target_link_libraries(smtkPQComponentsExt LINK_PUBLIC
  pqApplicationComponents
  smtkQtExt
  ${qt_targets}
)

if (SMTK_ENABLE_TESTING)
  target_link_libraries(TestBuild_smtk_extension_paraview_appcomponents
    LINK_PUBLIC
    smtkPQComponentsExt
    )
endif()

smtk_export_header(smtkPQComponentsExt Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPQComponentsExt)

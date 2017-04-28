smtk_find_package_qt(qt_targets REQUIRED
  QT4_COMPONENTS QtCore QtGui
  QT5_COMPONENTS Core Widgets)

find_package( VTK REQUIRED )
include_directories(${VTK_INCLUDE_DIRS})

set(OpenCVOperatorsHeaders imageFeatureExtractorWidget.h)
set(OpenCVOperatorsSrcs imageFeatureExtractorWidget.cxx)
set(OpenCVOperatorsUI imageFeatureExtractor.ui)

#install the headers
smtk_public_headers(${OpenCVOperatorsHeaders})

smtk_qt_wrap_cpp(MOC_BUILT_SOURCES ${OpenCVOperatorsHeaders})
smtk_qt_wrap_ui(UI_BUILT_SOURCES ${OpenCVOperatorsUI})

source_group("Qt_Source" FILES ${OpenCVOperatorsSrcs} ${OpenCVOperatorsUI})
source_group("Qt_Generated" FILES ${MOC_BUILT_SOURCES} ${UI_BUILT_SOURCES} ${RCS_RESOURCES})
source_group("Qt_Header" FILES ${OpenCVOperatorsMocHeaders})

add_library(smtkOpenCVQtExt
  ${OpenCVOperatorsSrcs}
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${RCS_RESOURCES}
)

target_include_directories(smtkOpenCVQtExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(smtkOpenCVQtExt LINK_PUBLIC
  pqApplicationComponents
  smtkCore
  smtkPQWidgetsExt
  vtkPolygonOperatorsExt
  vtkSMTKSourceExt
  vtkDomainsChemistryOpenGL2
  vtkSMTKFilterExt
  ${qt_targets}
LINK_PRIVATE
  ${VTK_LIBRARIES}
)

smtk_export_header(smtkOpenCVQtExt Exports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkOpenCVQtExt)

if (SMTK_ENABLE_TESTING)
  # Link *everything* in this directory to VTK modules
  # This is required for the TestBuild targets.
  # We must test whether each of the targets below exist
  # because they are required when built into VTK but
  # may not be present, especially since the OpenGL2
  # backend became available.
  set(SMTK_VTK_TARGET_LIST)
  foreach(target
    vtkInteractionStyle
    vtkRenderingFreeType
    vtkRenderingOpenGL2
    ${qt_targets}
    )
    if (TARGET ${target})
      list(APPEND SMTK_VTK_TARGET_LIST ${target})
    endif()
  endforeach()

  target_link_libraries(TestBuild_smtk_extension_opencv_qt PUBLIC ${SMTK_VTK_TARGET_LIST}
    smtkQtExt)
endif()

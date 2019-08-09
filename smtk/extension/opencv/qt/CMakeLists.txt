find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

find_package( VTK REQUIRED )
include_directories(${VTK_INCLUDE_DIRS})

set(OpenCVOperationsHeaders imageFeatureExtractorWidget.h)
set(OpenCVOperationsSrcs imageFeatureExtractorWidget.cxx)
set(OpenCVOperationsUI imageFeatureExtractor.ui)

#install the headers
smtk_public_headers(smtkOpenCVQtExt ${OpenCVOperationsHeaders})

set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTOUIC 1)

add_library(smtkOpenCVQtExt
  ${OpenCVOperationsSrcs}
  ${OpenCVOperationsHeaders}
  ${OpenCVOperationsUI}
  ${RCS_RESOURCES}
)

target_include_directories(smtkOpenCVQtExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(smtkOpenCVQtExt LINK_PUBLIC
  pqApplicationComponents
  smtkCore
  smtkPQWidgetsExt
  vtkPolygonOperationsExt
  vtkSMTKSourceExt
  vtkDomainsChemistryOpenGL2
  vtkSMTKFilterExt
  Qt5::Core
  Qt5::Widgets
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
    Qt5::Core
    Qt5::Widgets
    )
    if (TARGET ${target})
      list(APPEND SMTK_VTK_TARGET_LIST ${target})
    endif()
  endforeach()
endif()

include(${PARAVIEW_USE_FILE})
include(ParaViewPlugins)

# We need to add the current value of VTK_MODULES_DIR to the module path
# so that when the plugins are built all the modules can be found. Otherwise,
# modules that aren't loaded as direct dependencies of CMB modules will
# not be found.
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

IF (PARAVIEW_QT_VERSION VERSION_GREATER "4")
  QT5_WRAP_CPP(MOC_SRCS qtPolygonEdgeOperationView.h)
  QT5_WRAP_CPP(MOC_SRCS qtExtractContoursView.h)
  if(SMTK_ENABLE_OPENCV)
    QT5_WRAP_CPP(MOC_SRCS qtSurfaceExtractorView.h)
  endif()
#    QT5_WRAP_UI(UI_SRCS qtPolygonEdgeOperationView.ui)
ELSE ()
  QT4_WRAP_CPP(MOC_SRCS qtPolygonEdgeOperationView.h)
  QT4_WRAP_CPP(MOC_SRCS qtExtractContoursView.h)
  if(SMTK_ENABLE_OPENCV)
    QT4_WRAP_CPP(MOC_SRCS qtSurfaceExtractorView.h)
  endif()
#    QT4_WRAP_UI(UI_SRCS qtPolygonEdgeOperationView.ui)
ENDIF ()

ADD_SMTK_UI_VIEW(
  OUTIFACES
  OUTSRCS
  CLASS_NAME qtPolygonEdgeOperationView
  VIEW_NAME smtkPolygonEdgeView
  )
ADD_SMTK_UI_VIEW(
  OUTIFACES1
  OUTSRCS1
  CLASS_NAME qtExtractContoursView
  VIEW_NAME smtkPolygonContourView
  )

set(OpenCVSrc)
if(SMTK_ENABLE_OPENCV)
  ADD_SMTK_UI_VIEW(
    OUTIFACES
    OUTSRCS
    CLASS_NAME qtSurfaceExtractorView
    VIEW_NAME smtkSurfaceExtractorView
    )
  set(OpenCVSrc qtSurfaceExtractorView.cxx)
endif()



add_paraview_plugin(
  smtkPolygonSessionPlugin "1.0"
  SERVER_MANAGER_XML smconfig.xml
  SERVER_SOURCES sessionInit.cxx
  GUI_INTERFACES ${OUTIFACES} ${OUTIFACES1}
  GUI_SOURCES ${OUTSRCS} ${OUTSRCS1} ${MOC_SRCS} ${UI_SRCS}
              qtPolygonEdgeOperationView.cxx
              qtExtractContoursView.cxx
              ${OpenCVSrc}
  CS_KITS
    vtkSMTKWidgetsExt
    vtkPolygonOperatorsExt
  )

target_link_libraries(smtkPolygonSessionPlugin
  LINK_PUBLIC
    smtkCore
    smtkPolygonQtExt
    smtkPolygonSession
    smtkQtExt
    smtkPQWidgetsExt
    vtkPVServerManagerApplication
  LINK_PRIVATE
    vtkPVServerManagerApplicationCS
    vtkSMTKWidgetsExtCS
    vtkPolygonOperatorsExtCS
    vtkSMTKFilterExtCS
)
smtk_install_library(smtkPolygonSessionPlugin)

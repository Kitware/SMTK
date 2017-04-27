include(${PARAVIEW_USE_FILE})
include(ParaViewPlugins)

# We need to add the current value of VTK_MODULES_DIR to the module path
# so that when the plugins are built all the modules can be found. Otherwise,
# modules that aren't loaded as direct dependencies of CMB modules will
# not be found.
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

smtk_qt_wrap_cpp(MOC_SRCS qtPolygonEdgeOperationView.h)
smtk_qt_wrap_cpp(MOC_SRCS qtExtractContoursView.h)
#smtk_qt_wrap_cpp(UI_SRCS qtPolygonEdgeOperationView.ui)

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

add_paraview_plugin(
  smtkPolygonSessionPlugin "1.0"
  SERVER_MANAGER_XML smconfig.xml
  SERVER_SOURCES sessionInit.cxx
  GUI_INTERFACES ${OUTIFACES} ${OUTIFACES1} ${OUTIFACES2}
  GUI_SOURCES ${OUTSRCS} ${OUTSRCS1} ${OUTSRCS2} ${MOC_SRCS} ${UI_SRCS}
              qtPolygonEdgeOperationView.cxx
              qtExtractContoursView.cxx
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

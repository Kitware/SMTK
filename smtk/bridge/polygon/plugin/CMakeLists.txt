include(${PARAVIEW_USE_FILE})
include(ParaViewPlugins)

# We need to add the current value of VTK_MODULES_DIR to the module path
# so that when the plugins are built all the modules can be found. Otherwise,
# modules that aren't loaded as direct dependencies of CMB modules will
# not be found.
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

IF (PARAVIEW_QT_VERSION VERSION_GREATER "4")
  QT5_WRAP_CPP(MOC_SRCS qtPolygonEdgeOperationView.h)
#    QT5_WRAP_UI(UI_SRCS qtPolygonEdgeOperationView.ui)
ELSE ()
  QT4_WRAP_CPP(MOC_SRCS qtPolygonEdgeOperationView.h)
#    QT4_WRAP_UI(UI_SRCS qtPolygonEdgeOperationView.ui)
ENDIF ()

ADD_SMTK_UI_VIEW(
  OUTIFACES
  OUTSRCS
  CLASS_NAME qtPolygonEdgeOperationView
  VIEW_NAME smtkPolygonEdgeView
  )

add_paraview_plugin(
  smtkPolygonSessionPlugin "1.0"
  SERVER_MANAGER_XML smconfig.xml
  SERVER_SOURCES sessionInit.cxx
  GUI_INTERFACES ${OUTIFACES}
  GUI_SOURCES ${OUTSRCS} ${MOC_SRCS} ${UI_SRCS}
              qtPolygonEdgeOperationView.cxx
  CS_KITS vtkSMTKWidgetsExt
  )

target_link_libraries(smtkPolygonSessionPlugin
  LINK_PUBLIC
    smtkCore
    smtkPolygonQtExt
    smtkPolygonSession
    smtkQtExt
    vtkPVServerManagerApplication
  LINK_PRIVATE
    vtkPVServerManagerApplicationCS
    vtkSMTKWidgetsExtCS
)
smtk_install_library(smtkPolygonSessionPlugin)

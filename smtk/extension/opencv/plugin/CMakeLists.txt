include(${PARAVIEW_USE_FILE})
include(ParaViewPlugins)

# We need to add the current value of VTK_MODULES_DIR to the module path
# so that when the plugins are built all the modules can be found. Otherwise,
# modules that aren't loaded as direct dependencies of CMB modules will
# not be found.
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

smtk_qt_wrap_cpp(MOC_SRCS qtSurfaceExtractorView.h)

ADD_SMTK_UI_VIEW(
  OUTIFACES
  OUTSRCS
  CLASS_NAME qtSurfaceExtractorView
  VIEW_NAME smtkSurfaceExtractorView
  )
set(OpenCVSrc qtSurfaceExtractorView.cxx)

add_paraview_plugin(
  smtkOpenCVPlugin "1.0"
  SERVER_SOURCES sessionInit.cxx
  GUI_INTERFACES ${OUTIFACES}
  GUI_SOURCES ${OUTSRCS} ${MOC_SRCS} ${UI_SRCS}
              ${OpenCVSrc}
  CS_KITS
    vtkSMTKWidgetsExt
    vtkSMTKOpenCVExt
  )

target_link_libraries(smtkOpenCVPlugin
  LINK_PUBLIC
    smtkCore
    smtkOpenCVExt
    smtkOpenCVQtExt
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
smtk_install_library(smtkOpenCVPlugin DEPENDS smtkOpenCVExt)

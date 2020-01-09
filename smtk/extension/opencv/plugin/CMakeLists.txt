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
  GUI_SOURCES ${OUTSRCS} ${UI_SRCS}
              ${OpenCVSrc}
              qtSurfaceExtractorView.h
  CS_KITS
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
  LINK_PRIVATE
    vtkPVServerManagerApplicationCS
    vtkSMTKWidgetsExtCS
    vtkPolygonOperationsExtCS
    vtkSMTKFilterExtCS
)
smtk_install_library(smtkOpenCVPlugin DEPENDS smtkOpenCVExt)

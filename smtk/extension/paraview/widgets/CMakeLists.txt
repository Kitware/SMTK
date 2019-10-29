set(sources
  pqConePropertyWidget.cxx
  pqConePropertyWidget.h
  pqPointPropertyWidget.cxx
  pqPointPropertyWidget.h
  pqSMTKAttributeItemWidget.cxx
  pqSMTKAttributeItemWidget.h
  pqSMTKAttributeItemWidgetP.h
  pqSMTKBoxItemWidget.cxx
  pqSMTKBoxItemWidget.h
  pqSMTKConeItemWidget.cxx
  pqSMTKConeItemWidget.h
  pqSMTKInfiniteCylinderItemWidget.cxx
  pqSMTKInfiniteCylinderItemWidget.h
  pqSMTKLineItemWidget.cxx
  pqSMTKLineItemWidget.h
  pqSMTKPlaneItemWidget.cxx
  pqSMTKPlaneItemWidget.h
  pqSMTKPointItemWidget.cxx
  pqSMTKPointItemWidget.h
  pqSMTKSphereItemWidget.cxx
  pqSMTKSphereItemWidget.h
  pqSMTKSplineItemWidget.cxx
  pqSMTKSplineItemWidget.h
  pqSMTKWidgetsAutoStart.cxx
  pqSMTKWidgetsAutoStart.h)

set(ui_files
  resources/pqConePropertyWidget.ui
  resources/pqPointPropertyWidget.ui)
set(CMAKE_AUTOUIC_SEARCH_PATHS
  "${CMAKE_CURRENT_SOURCE_DIR}/resources")

set(rc_files
  qtWidgetsIcons.qrc)

set(views
  qtSimpleExpressionEvaluationView)
set(interfaces)
foreach (view IN LISTS views)
  smtk_plugin_add_ui_view(
    CLASS_NAME "${view}"
    VIEW_NAME "${view}"
    INTERFACES view_interfaces
    SOURCES view_sources)
  list(APPEND interfaces
    ${view_interfaces})
  list(APPEND sources
    "${view}.cxx"
    "${view}.h"
    ${view_sources})
endforeach ()

paraview_plugin_add_auto_start(
  CLASS_NAME pqSMTKWidgetsAutoStart
  INTERFACES auto_start_interfaces
  SOURCES auto_start_sources)
list(APPEND interfaces
  ${auto_start_interfaces})
list(APPEND sources
  ${auto_start_sources})

paraview_add_plugin(smtkPQWidgetsPlugin
  VERSION "1.0"
  UI_INTERFACES ${interfaces}
  UI_FILES ${ui_files}
  UI_RESOURCES ${rc_files}
  SOURCES ${sources})
target_link_libraries(smtkPQWidgetsPlugin
  PRIVATE
    ParaView::pqApplicationComponents
    smtkQtExt
    vtkSMTKWidgetsExt)

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
  pqSMTKWidgetsAutoStart.h
  qtSimpleExpressionEvaluationView.cxx
  qtSimpleExpressionEvaluationView.h
  Registrar.cxx
  Registrar.h)

set(ui_files
  resources/pqConePropertyWidget.ui
  resources/pqPointPropertyWidget.ui)
set(CMAKE_AUTOUIC_SEARCH_PATHS
  "${CMAKE_CURRENT_SOURCE_DIR}/resources")

set(rc_files
  qtWidgetsIcons.qrc)

# for Qt methods
set(CMAKE_AUTOMOC 1)

paraview_plugin_add_auto_start(
  CLASS_NAME pqSMTKWidgetsAutoStart
  INTERFACES auto_start_interfaces
  SOURCES auto_start_sources)
list(APPEND interfaces
  ${auto_start_interfaces})
list(APPEND sources
  ${auto_start_sources})

smtk_add_plugin(smtkPQWidgetsPlugin
  REGISTRAR
    smtk::extension::paraview::widgets::Registrar
  MANAGERS
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0"
    UI_INTERFACES ${interfaces}
    UI_FILES ${ui_files}
    UI_RESOURCES ${rc_files}
    SOURCES ${sources})
target_link_libraries(smtkPQWidgetsPlugin
  PRIVATE
    ParaView::pqApplicationComponents
    ParaView::RemotingViews
    smtkQtExt
    vtkSMTKWidgetsExt)

smtk_export_header(smtkPQWidgetsPlugin Exports.h)

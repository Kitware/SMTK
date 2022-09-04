set(sources
  pqSMTKWidgetsAutoStart.cxx
  pqSMTKWidgetsAutoStart.h
)

# for Qt methods
set(CMAKE_AUTOMOC 1)

paraview_plugin_add_auto_start(
  CLASS_NAME pqSMTKWidgetsAutoStart
  INTERFACES auto_start_interfaces
  SOURCES auto_start_sources
)
list(APPEND interfaces
  ${auto_start_interfaces}
)
list(APPEND sources
  ${auto_start_sources}
)

set(smxml_files
  pqSlicePropertyWidget.xml
)

smtk_add_plugin(smtkPQWidgetsPlugin
  REGISTRAR
    smtk::extension::paraview::widgets::Registrar
  MANAGERS
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0"
    REQUIRED_PLUGINS smtkPQComponentsPlugin
    UI_INTERFACES ${interfaces}
    SOURCES ${sources}
    SERVER_MANAGER_XML ${smxml_files}
    MODULES smtkPQWidgetsExt
)
target_link_libraries(smtkPQWidgetsPlugin
  PRIVATE
    vtkSMTKWidgetsExt
    smtkQtExt
    smtkPQWidgetsExt
    ParaView::pqApplicationComponents
    ParaView::RemotingViews
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::CommonExecutionModel
    VTK::CommonMath
    VTK::CommonTransforms
    VTK::InteractionWidgets
    VTK::RenderingCore
    VTK::RenderingOpenGL2
)

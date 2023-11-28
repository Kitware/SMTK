set(sources
  pqSMTKMarkupAutoStart.cxx
  pqSMTKMarkupAutoStart.h
)

# for Qt methods
set(CMAKE_AUTOMOC 1)

paraview_plugin_add_auto_start(
  CLASS_NAME pqSMTKMarkupAutoStart
  INTERFACES auto_start_interfaces
  SOURCES auto_start_sources
)
list(APPEND interfaces
  ${auto_start_interfaces}
)
list(APPEND sources
  ${auto_start_sources}
)

get_property(smtk_vtk_modules GLOBAL
  PROPERTY _smtk_vtk_extention_modules)

smtk_add_plugin(smtkPQMarkupPlugin
  REGISTRARS
    smtk::extension::paraview::markup::Registrar
    smtk::extension::vtk::markup::Registrar
  MANAGERS
    smtk::geometry::Manager
    smtk::operation::Manager
    smtk::resource::query::Manager
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION 1.0
    REQUIRED_ON_SERVER
    REQUIRED_ON_CLIENT
    UI_INTERFACES ${interfaces}
    SOURCES ${sources}
    MODULES smtkPQMarkupExt
)
target_link_libraries(smtkPQMarkupPlugin
  PRIVATE
    vtkSMTKMarkupExt
    smtkPQMarkupExt
    smtkQtExt
    smtkMarkup
    # ParaView::pqApplicationComponents
    # ParaView::RemotingViews
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::CommonExecutionModel
    VTK::CommonMath
    VTK::CommonTransforms
    VTK::InteractionWidgets
    VTK::RenderingCore
    VTK::RenderingOpenGL2
)

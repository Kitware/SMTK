set(classes
  Registrar
  pqJobRunnerView
)

vtk_module_add_module(smtkPVJobExt
  CLASSES ${classes}
  HEADERS_SUBDIR "smtk/extension/paraview/job"
)

qt_wrap_cpp(
  QTMOCSRCS
  pqJobRunnerView.h
  TARGET smtkPVJobExt
)
set_property(SOURCE pqJobRunnerView.cxx PROPERTY OBJECT_DEPENDS "${QTMOCSRCS}")

vtk_module_link(smtkPVJobExt
  PUBLIC
    VTK::CommonDataModel
    smtkCore
    smtkQtExt
    smtkPQComponentsExt
    ParaView::qttesting
    ParaView::pqComponents
)

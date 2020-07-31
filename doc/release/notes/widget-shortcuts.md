+ A change in ParaView's pqPointPickingHelper has removed the
  setShortcutEnabled method. If you were previously connecting
  this method to a pqInteractivePropertyWidget subclass (like
  the cone widget in SMTK), replace that signal connection
  with an instantiation of pqPointPickingVisibilityHelper.
  See `smtk/extension/paraview/widgets/plugin/pqConePropertyWidget.cxx`
  for an example of its usage.

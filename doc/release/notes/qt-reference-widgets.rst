Fixes for ReferenceItem widgets
-------------------------------

Neither :smtk:`smtk::extension::qtReferenceItem`
nor :smtk:`smtk::extension::qtReferenceTree` properly called
their owning-view's ``valueChanged()`` method when modified.
Now they do. If you had to work around this issue before, be
aware that you may have to undo those workarounds to avoid
double-signaling.

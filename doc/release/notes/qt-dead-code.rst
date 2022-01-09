Removal of qtActiveObjects
--------------------------

The ``qtActiveObjects`` class is no longer in use anywhere and has been removed.
If you used it in a third-party extension, you'll need to create a new object
to hold an ``smtk::view::Selection::Ptr`` and an active ``smtk::model::Model``.

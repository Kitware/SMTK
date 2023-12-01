Graph system
------------

Run-time arcs
~~~~~~~~~~~~~

Graph resources now support the run-time creation of arc types.
This is implemented by making the :smtk:`smtk::graph::ArcImplementation` template
inherit and implement a pure virtual :smtk:`smtk::graph::ArcImplementationBase` class.
Arc types may be created via a new :smtk:`smtk::graph::CreateArcType` operation
and arcs of any type may be created via the :smtk:`smtk::graph::CreateArc` operation.

See the :ref:`smtk-qt-sys` documentation for user-interface elements that support
arc creation and deletion.

Graph arc storage
~~~~~~~~~~~~~~~~~

In order to support run-time arc types, the :smtk:`smtk::graph::ArcMap` class
no longer inherits :smtk:`smtk::common::TypeContainer`.
Instead, it owns an unordered map from string tokens to shared pointers to
:smtk:`smtk::graph::ArcImplementationBase`.
The :smtk:`smtk::graph::ArcImplementation` template inherits this base type
in order to provide virtual-method access to arcs (in addition to the high
speed interface unique to the arc traits object).

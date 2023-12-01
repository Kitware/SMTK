Views
=====

As mentioned in the previous section, views may show
editable attribute resources
or
trees of descriptive phrases.
Configuration information is different for each of these types.

Attribute views
---------------

This view information is used to configure :smtk:`qtBaseView` instances.
Views of an attribute resource may need to display many attributes, some of
which may be created upon user input (i.e., by adding a new material or boundary
condition).
However, there must be one view marked as the top-level view for any given
attribute resource.
This view may have child views that it uses to organize
(1) fixed, one-per-simulation attribute instances as well as
(2) attribute instances that may vary in number to fit the needs of the simulation.

Phrase views
------------

These views appear in 2 types of widgets in SMTK:
panels holding tree views (such as the :smtk:`resource panel <smtk::extension::qtResourceBrowser>`)
and widgets for selecting components or resources, where only a flat list is presented
(such as the :smtk:`reference item widget <smtk::extension::qtReferenceItem>`).

Diagram views
-------------

A :smtk:`diagram <smtk::extension::qtDiagram>` is a schematic view that
uses nodes and arcs to represent the organization of information in a
two-dimensional graph.
This type of view is entirely dependent on Qt and further documentation
for it is in the :ref:`smtk-qt-sys` extension section.

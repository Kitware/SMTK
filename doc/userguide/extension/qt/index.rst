.. _smtk-qt-sys:

Qt
==

SMTK provides Qt classes that allow users to edit attributes and inspect model and mesh resources.
Most of the Qt support library is focused on providing Qt implementations of the
:smtk:`smtk::view::BaseView` class so that users can view and edit attribute resources.

* :smtk:`qtBaseView <smtk::extension::qtBaseView>` – a base class for all views implemented with Qt
* :smtk:`qtAttributeView <smtk::extension::qtAttributeView>` – a view that allows users to create
  and destroy attributes that inherit a designer-specified set of
  :smtk:`Definition <smtk::attribute::Definition>` instances.
* :smtk:`qtInstancedView <smtk::extension::qtInstancedView>` – a view that allows users to edit
  designer-specified :smtk:`Attribute <smtk::attribute::Attribute>` instances.
* :smtk:`qtAssociationView <smtk::extension::qtAssociationView>` – a view that allows users to edit
  the objects associated to an attribute or collection of attributes.
* :smtk:`qtResourceBrowser <smtk::extension::qtResourceBrowser>` – a view that arranges
  persistent objects into a tree.

Besides Qt-based attribute and phrase views that use traditional widgets,
SMTK also provides a facility for schematic diagrams discussed below.

.. toctree::
   :maxdepth: 3

   concepts.rst
   attribute-views.rst
   diagram.rst

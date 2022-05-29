.. _release-notes-22.05:

=========================
SMTK 22.05 Release Notes
=========================

See also :ref:`release-notes-22.04` for previous changes.


SMTK Resource Changes
========================

New methods on smtk::resource::Resource
---------------------------------------

You can now ask a resource for raw pointers to components
via the ``components()`` and the (templated) ``componentsAs<X>()``
methods.
Use these methods **only** when

(1) you have a read or write lock on the resource (i.e., you are
    inside an operation) so that no other thread can remove the
    component; and
(2) you are sure that the component will not be removed from the
    resource for the duration of your use of the pointer (in the
    case where a write lock is held and components may be removed).

Note for developers
~~~~~~~~~~~~~~~~~~~

If you subclass :smtk:`smtk::resource::Resource`, you should
consider overriding the ``components()`` method to provide an
efficient implementation as it can improve performance as the
number of components grows large.
You may also wish to consider changing any subclasses of
:smtk:`smtk::resource::Component` to refer back to their parent
resource using a :smtk:`smtk::common::WeakReferenceWrapper`
and providing a method to fetch the raw pointer to the parent
resource from the weak-reference-wrapper's cache.
This will improve performance when many components must be asked
for their parent resource as the overhead of locking a weak pointer
and copying a shared pointer can be significant.
However, note that any method returning a raw pointer will not
be thread-safe. This method is intended solely for cases where
a read- or write-lock is held on the resource and the algorithm
can guarantee a shared pointer is held elsewhere for the duration.

If you maintain an operation that needs to be performant with
large numbers of components, consider using these methods to
avoid the overhead of shared-pointer construction.


SMTK Attribute Resource Changes
===============================

Allowing Designers to Set the Default Name Separator for Attribute Resources
----------------------------------------------------------------------------

Added an API to set, get and reset the Default Name Separator used by the Attribute Resource when creating unique names
for a new Attribute.  Also added support in Version 5 XML Attribute Files as well as in JSON representations to save and
retrieve the separator.

New C++ API Changes for smtk::attribute::Resource
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: c++

  /**\brief Get the separator used for new Attributes whose names are not unique
   */
  const std::string &defaultNameSeparator();
  /**\brief Reset the separator used for new Attributes whose names are not unique to to the default which is '-'.
   */
  void resetDefaultNameSeparator();
  /**\brief Set the separator used for new Attributes whose names are not unique
   */
  bool setDefaultNameSeparator(const std::string& separator);

XML Formatting Additions
~~~~~~~~~~~~~~~~~~~~~~~~

This example sets the Name Separator to **::**

.. code-block:: xml

<SMTK_AttributeResource Version="5" ID="504c3ea1-0aa4-459f-8267-2ba973d786ad" NameSeparator="::">
</SMTK_AttributeResource>

Attribute itemPath Method
-------------------------

Added "smtk::attribute::Attribute::itemPath" method that will return the full path string to the "item"
parameter within the attribute. The default separator is "/" and can be
changed as needed.

SMTK View Changes
=================

Descriptive Phrase Model Change in Subphrase Generation
--------------------------------------------------------

Previously, subphrases generation was done on demand as a performance optimization.  Unfortunately, this made certain potential functionality such as ternary visibility difficult to implement at best.  By forcing the Phrase Model to build all that it can in terms of its subphrases, it will now be possible to calculate and maintain a ternary visibility state.

SMTK QT Changes
================

Saving Collapsible Group View Box State
---------------------------------------

The View now stores its open/close state within its configuration so
when it is rebuilt it knows if it should be open or closed.

Designers can also use this to set the initial state of the Group View.

The XML Attribute that controls this is called **Open** and is set to true or false.


Qt subsystem: badge click actions
---------------------------------

Previously, the :smtk:`smtk::extension::qtDescriptivePhraseDelegate`
generated badge clicks inside its ``editorEvent()`` method.
However, that method is not provided with access to the view in
which the click occurred and thus could not access the view's
QSelectionModel.
Now, each widget that uses the descriptive-phrase delegate
(:smtk:`smtk::extension::qtResourceBrowser`,
:smtk:`smtk::extension::qtReferenceItem`,
:smtk:`smtk::extension::qtReferenceTree`) installs an
event filter on its view's viewport-widget and passes
mouse clicks to ``qtDescriptivePhraseDelegate::processBadgeClick()``.

User-facing changes
~~~~~~~~~~~~~~~~~~~

Users will now see that:

+ clicks on badges in the widgets above will not change the Qt
  selection as they did previously,
+ clicking on the badge of a selected item in the widgets above
  will act upon all the selected items, and
+ clicking on the badge of an unselected item in the widgets above
  will only act on that item.

This behavior should be significantly more intuitive than before.

Developer notes
~~~~~~~~~~~~~~~

If you use qtDescriptivePhraseDelegate in any of your own
classes, you cannot rely on it to handle badge clicks itself;
instead you must install an event filter on your view-widget's
viewport (not the view widget itself) and pass mouse clicks
to the delegate along with the view widget.

Added onDefinitionChange Signal to qtAttributeView
--------------------------------------------------

Added a signal called "onDefinitionChange" that will emitted when the user
selects a definition from the definition combo box. Classes that inherit
qtAttributeView can connect to this signal in order control the widget's
behavior based on the type of definition the user has selected.

qtGroupItem's Modified Children
-------------------------------
When the user modifies an item that is a child of a group item from the gui,
the full path to that item is included in the result of the resulting Signal
operation that will be run by the view.

qtInputsItem children displayed on right
----------------------------------------

The children of a discrete qtInputsItem widget are now displayed to the right
of the combobox, rather than underneath the combobox. This change was made to
decrease wasted space in the Attribute Panel, but also to remove the primary
example case of a Qt-related bug that could squish the child input widgets
when the parent was extensible.

Developer changes
~~~~~~~~~~~~~~~~~~

This makes the Attribute Panel wider than it was before when a qtInputsItem
with children is present (but also shorter). Although this isn't deemed to
be a problem, it's something to be aware of.

User-facing changes
~~~~~~~~~~~~~~~~~~~

When a discrete value combobox had children, they used to display beneath the
combobox. With this change, the children will now display to the right of the combobox.
See `here <https://gitlab.kitware.com/cmb/smtk/-/merge_requests/2738>`_ for more details.


SMTK ParaView Extensions Changes
================================

Reorganization of extensions/paraview/appcomponents
---------------------------------------------------

To benefit client apps of smtk, the sources and resources in
`extensions/paraview/appcomponents/plugin-core` have all been moved
to the `appcomponents` directory and into the `smtkPQComponentsExt`
library, so they are accessible outside smtk. The plugin has been
split into three, with `plugin-core` retaining the auto-start and
behaviors, and `plugin-gui` containing all the toolbars, and panels,
and `plugin-readers` containing the importers and readers.

SMTK VTK Related Changes
========================

Dataset information operation for VTK backend
---------------------------------------------

There is a new operation (:smtk:`smtk::geometry::DataSetInfoInspector`)
that computes, for each associated component, the number of points and
cells (of each type) present in the VTK-renderable geometry.

This operation has a custom view that automatically updates a table showing
the counts as soon as the associations change.


VTK resource multi-block source changes
---------------------------------------

If an :smtk:`smtk::geometry::Geometry` provider adds the name
of a component to a cache entry (by setting the ``vtkCompositeDataSet::NAME()``
key the cache entry's ``vtkInformation`` object), the
:smtk:`vtkSMTKResourceMultiBlockSource` filter will copy the information
key to its output.

If you are an end user of a ParaView-based application, then the information
panel may now display more helpful information in its block-inspector tree.

If you maintain your own geometry cache for a custom resource type and wish
users to see the name in ParaView's (and/or ModelBuilder's) information panel,
then you should update your geometry object to set the name.


SMTK Graph Session Changes
==========================

Breaking changes to graph-resource arcs
---------------------------------------

The way SMTK represents arcs has changed.
This is a breaking change to accommodate new functionality:
the ability to provide or request indexing of both the
"forward" arc direction (from, to) and the "reverse" arc
direction (to, from).

Previously, the :smtk:`ArcMap <smtk::graph::ArcMap>` class held arcs
in a :smtk:`TypeMap <smtk::graph::TypeMap>` (a map of maps from
arc type-name to UUID to arc API object).
Now the ArcMap is a :smtk:`TypeContainer <smtk::graph::TypeContainer>`
(a simple map from arc ``typeid`` hash code to arc API object).

The arc classes have also changed.
If you previously did not inherit any of SMTK's arc classes,
you will need to adapt your own arc classes to expose new
methods. See the graph-session documentation for the new
method(s) you must provide.
If you previously inherited Arc, Arcs, or OrderedArcs in your
resource, these classes are removed.
Instead, if you do not provide implementations of methods for
accessing and manipulating arcs, the
:smtk:`implementation <smtk::graph::ArcImplementation>` will provide
them for you by applying the
:smtk:`ExplicitArcs <smtk::graph::ExplicitArcs>` template to
your traits class.

For more details, see the updated documentation for the
graph subsystem.

Python Related Changes
======================

Python bindings for smtk.common
-------------------------------

Now `smtk.common.Managers` has python bindings.
Without this class being wrapped, it was impossible
to call `smtk.resource.Manager.read` and `smtk.resource.Manager.write`
since these methods now require a Managers instance.


Plugin Changes
==============

Make QT_NO_KEYWORDS optional for external plugins
-------------------------------------------------

External plugins can add the option `ALLOW_QT_KEYWORDS` to their use of
`smtk_add_plugin` to skip the enforcement of `QT_NO_KEYWORDS` for
the plugin. For example:

.. code-block:: make
smtk_add_plugin(
  smtkAEVASessionPlugin
  ALLOW_QT_KEYWORDS
  PARAVIEW_PLUGIN_ARGS
  VERSION "1.0"
  ...)



General Build Changes
=====================

Support for old hash namespace conventions dropped
--------------------------------------------------

SMTK now requires compilers to support specializations
of hash inside the ``std`` namespace.
Previously, SMTK accepted specializations in non-standard
locations such as the global namespace, in the ``std::tr1``
namespace, and others.

Developer notes
~~~~~~~~~~~~~~~

If your repository uses any of the following macros, use the
replacements as described:

+ ``SMTK_HASH_NS`` should be replaced with ``std``
+ ``SMTK_HASH_NS_BEGIN`` should be replaced with ``namespace std {``
+ ``SMTK_HASH_NS_END`` should be replaced with ``}``
+ Any code enabled by ``SMTK_HASH_SPECIALIZATION`` should be unconditionally
  enabled and any code disabled by this macro should be removed.

.. _release-notes-22.07:

=========================
SMTK 22.07 Release Notes
=========================

See also :ref:`release-notes-22.05` for previous changes.


SMTK Common Changes
========================

RGBA color conversion
---------------------

Now :smtk:`smtk::common::Color` includes methods to convert 4-tuples of floating-point
numbers into hexadecimal color specifiers (in addition to the 3-tuples it already handles).


SMTK Attribute Resource Changes
===============================

Expanding SMTK Attribute Category Mechanism
-------------------------------------------

Category Modeling in SMTK Attribute Resources has been enhanced to now support specialization as well as generalization.
Previously, an Attribute or Item Definition's local categories could only expand (make more general) the categories it was inheriting (or choose to ignore them all together).
With this release, SMTK now supports specializing (or making the category constraint more restrictive).

Previously category inheritance was controlled by using the Definition's setIsOkToInherit method which would either **Or** its local categories with those it was inheriting or **Replace** them. This method (as well as isOkToInherit method) has been deprecated.  The new methods for setting and retrieving category inheritance are:

* setCategoryInheritanceMode
* categoryInheritanceMode

The values that categoryInheritanceMode can be set to are:

* smtk::attribute::Categories::CombinationMode::Or
* smtk::attribute::Categories::CombinationMode::And
* smtk::attribute::Categories::CombinationMode::LocalOnly

Setting the mode to **Or** is the same as the previous setIsOkToInherit(true) - it will **or** the Definition's local categories with those that it is inheriting. Setting the mode to **And** will now **and** the Definition's local categories with those that it is inheriting.  Setting the mode to **LocalOnly** will ignore the categories that the Definition is inheriting and is the same as the previous setIsOkToInherit(false).

Changing the Default Category Inheritance
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Previously the default mode was isOkToInherit = true which now corresponds to smtk::attribute::Categories::CombinationMode::Or.  Upon discussing the new combination support of **And**, it was decided that the new default will be smtk::attribute::Categories::CombinationMode::And, meaning that a Definition will be more specialized category-wise than it's parent Definition.

Change on Enum Categories
~~~~~~~~~~~~~~~~~~~~~~~~~
Previously, categories on a Discrete Value Item Definition's Enums would add their categories to the Definition.  With this release it is assumed that the enums' categories will be **and'd** with it's Definition, there is no longer a reason for the enum categories to be combined with the Definition's local categories.

File Version Changes
~~~~~~~~~~~~~~~~~~~~
Supporting these changes did require a new format for both Attribute XML and JSON files.  The latest versions for both is now **6**.  Older file formats will load in properly and should work based on the previous category rules.

Developer changes
~~~~~~~~~~~~~~~~~~

The following methods and enums have been deprecated:

* smtk::attributeCategories::Set::CombinationMode::Any -> please use smtk::attributeCategories::Set::CombinationMode::Or
* smtk::attributeCategories::Set::CombinationMode::All -> please use smtk::attributeCategories::Set::CombinationMode::And
* smtk::attribute::Definition::setIsOkToInherit -> please use smtk::attribute::Definition::setCategoryInheritanceMode
* smtk::attribute::Definition::isOkToInherit -> please use smtk::attribute::Definition::categoryInheritanceMode
* smtk::attribute::ItemDefinition::setIsOkToInherit -> please use smtk::attribute::ItemDefinition::setCategoryInheritanceMode
* smtk::attribute::ItemDefinition::isOkToInherit -> please use smtk::attribute::ItemDefinition::categoryInheritanceMode

A new class for supporting the new combination modes has been developed called smtk::attribute::Categories::Stack which represents the category expression formed when combining inherited and local categories since we now need to maintain the order in which they are combined.

The method smtk::attribute::ValueItem:relevantEnums(bool includeCategories, bool includeReadAccess, unsigned int readAccessLevel) const was added in order to return the set of enums that passed the activce category and advance level checks (if specified).


SMTK QT Changes
================

Attribute items have a "Reset to Default" context menu item
-----------------------------------------------------------

All the widgets used to input values for attribute items have
a context menu item added called "Reset to Default", which will
change the value back to the default, if one is specified. If
no default is specified, the menu entry is disabled.

Int, Double, String, File, and DateTime items are all supported,
including spinbox and multi-line views. Resource items can't have
a default specified.

As a result, `qtInputItem` no longer set themselves back to the
default value when constructed - the default value is only
applied by the `ValueItem` when created, or when the user
chooses "Reset to Default".

qtItem changes
---------------

Several changes were made to the `qtItem` subclasses.

1. The new code hides the QLabel instance for an item if its label is
set to a (nonempty) blank string. Template writers use a whitespace
character to hide the label, however, a QFrame is still displayed and
takes up horizontal space. The new code essentially removes that unwanted
space.

2. The new code changes the Qt alignment for horizontal child-item layouts
from vertical-center to top, for aesthetic reasons.

3. The new code updates the logic for setting the layout direction
(horizontal or vertical) in `qtInputsItem` for various situations.

* For ordinary value items, the default layout is horizontal, but can be overridden by an ItemView `<View Layout="Vertical">`.
* For extensible items, the layout is *always* vertical, and cannot be overridden by an ItemView.
* For discrete value items with children items, the layout is either:

  * **horizontal** -  if each discrete value is assigned no child items or a
    single child item having a blank string for its label
  * **vertical** (otherwise)
  * The discrete item layout can be overridden by an item
    view, either `<View Layout="Horizontal">` or
    `<View Layout="Vertical">`.

Changes to qtInputsItem
--------------------------

A qtInputsItem instance will now call the Signal Operator when creating a new expression attribute.


SMTK ParaView Extensions Changes
================================

Panels separated from DockWidgets
---------------------------------

Previously, several `Panel` classes derived directly from `QDockWidget`,
so they could be docked by the user in whatever arrangement was desired.

To allow re-use and rearrangement, these `Panel` classes now derive from
`QWidget`, and are placed inside a `pqSMTKDock<T>` which derives from
`QDockWidget`. `pqSMTKDock<T>` has a template parameter to allow it to create the
child `Panel` of the correct type. `Panel` classes must now implement `void
setTitle(QString title)` to provide the `pqSMTKDock<T>` with the correct title,
and use `setWindowTitle()` to provide the initial dock window title.

ParaView resource panel
-----------------------

The :smtk:`pqSMTKResourcePanel` class now asks any :smtk:`smtk::view::ApplicationConfiguration`
present for view configuration before using a default. This makes it simpler for applications
to provide a custom phrase model, subphrase generator, or set of badges.
(Before this change, applications would have to wait for the panel to become ready and then
reconfigure it.)

Coloring renderable geometry
----------------------------

Before the property system was in wide use, the geometry
subsystem expected component colors to be passed via a
single-tuple field-data array on the renderable geometry.
Support for this was broken when support for coloring
by a floating-point property was added.

This commit fixes an issue (properly scaling floating-point
colors when generating an integer-valued color array) and
re-enables support for passing color by field-data.
Support for passing color by entity property is preserved,
but field-data arrays are preferred if present (because
presumably the geometry backend added this array).

This feature is being revived to support components inheriting
color from other components.

Resource panel search bar
-------------------------

A search bar has been added to the resource panel by default. Users may search
for resource or components using a wildcard text-based search. The search bar
is specified in the JSON config by adding `"SearchBar": true` to the
`Attributes` group.

Selection filtering for graph resources
---------------------------------------

A bug in the pqSMTKSelectionFilterBehavior class has been fixed.
It prevented the default selection-responder operation from adding
graph nodes to the selection (basically, anything that could not be
cast to a model entity was rejected).

Now the selection filter toolbar buttons only apply to model and
mesh entities; graph-resource components will always be passed
through.

SMTK Graph Session Changes
==========================

Graph-resource Dump improvements
--------------------------------

While :smtk:`smtk::graph::Resource`'s dump() method is unchanged,
if you use ``resource->evaluateArcs<Dump>()`` explicitly it is
now possible to set a color per arc type and to enable a
transparent background for the graph.
(This applies only to graphviz-formatted output.)

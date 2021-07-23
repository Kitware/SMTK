=========================
SMTK 21.07 Release Notes
=========================

See also `SMTK 21.05 Release Notes`_ for previous changes.

.. _`SMTK 21.05 Release Notes`: smtk-21.05.md

Registration Changes
====================
Manager/registry tracking
-------------------------

Tracking of registrations between registrars and managers should now only be
done through a held ``Registry`` instance as returned by the
``Registrar::addToManagers`` method. In the future, direct access to
``Registrar::registerTo`` and ``Registrar::unregisterFrom`` will be disabled
and the ``Registry`` RAII object will be the only way to manage registrations.
This ensures that registrations are accurately tracked.

SMTK Resource and Component Changes
===================================

Expanding Default Property Types on SMTK Resources/Components
-------------------------------------------------------------
The default set of Properties now include int, bool, std::vector<int> and std::vector<bool>.

Attribute Resource Changes
==========================
Preserve State Information
--------------------------

SMTK will now preserve state information that pertains to
Attribute Resource views.  The information persists even
after the resource has been closed, assuming the resource
was saved before it was closed.

Information Preserved:

- The active advance level.
- The active attributes in attribute views.
- The active tab in a group views.

Fix isValid Check w/r to an Attribute's Associations
----------------------------------------------------

If an attribute had the following conditions:

- Its association item set to have its NumberOfRequiredValues > 0
- Its Resource had active categories
- All of its items where validity set with the exception of its association item

Then its isValid() would mistakenly return true, due to the fact that its association item (which does not have categories set), would be excluded from the check.

Now the association item is forced to be by turning off category checking for that item.  By doing this, we are saying that if the attribute itself passes its category checks, then so does its association item.

.. highlight::cpp

.. highlight::cpp

Added the ability to ignore an item
===================================
There are times when a workflow may consider an item no longer relevant based on choices the user has made.  In order model this behavior two methods have been added to Item:

.. code-block:: c++

  void setIsIgnored(bool val);
  bool isIgnored() const;


If setIsIgnored is passed true, then the item's isRelevant() method will return false, regardless of any other facts.
This value is persistent and is supported in both JSON and XML formats.

Changes to Attribute and Item isRelevant Methods
------------------------------------------------

Both Attribute::isRelevant and Item::isRelevant have been modified to optional do advance level checking.

.. code-block:: c++

  bool isRelevant(bool includeCategoryChecks = true, bool includeReadAccess = false,
    int readAccessLevel = 0) const;

If includeCategoryChecks is set to true, then the Attribute or Item must pass their category checks based on the
resource's Active Category Settings.

If includeReadAccess is set to true then an Attribute is relevant iff at least one of it's children has a read access level <= readAccessLevel.

In the case of an Item, if includeReadAccess is true then it must  have it's read access level <= readAccessLevel

Note that this modification does not require any code change in order to preserve previous functionality.

Added hasRelevantChildren method to GroupItem
---------------------------------------------
GroupItem now has a method to test  if at least on of its children items passes their category checks and read access checks passed on the caller's requirements.

.. code-block:: c++

  bool hasRelevantChildren(bool includeCategoryChecks = true, bool includeReadAccess = false,
    int readAccessLevel = 0) const;

Changing ReferenceItem::AppendValue to Support Append Non-Unique
----------------------------------------------------------------
* Added a nonUnique parameter that defaults to false
* This avoids unnecessarily having to scan the entire item when duplicates are allowed
* Item now also tracks the location of the first unset value in order to speed up the append process

Model Resource Changes
======================
Model resource transcription
----------------------------

SMTK now provides a way to avoid an O(n^2) performance
issue when embedding many cells into a model;
previously, each insertion would perform a linear search
of pre-existing relationships. However, many operations
(especially those in the importer group) will not attempt
to re-insert existing relationships. The ``Model::addCell()``
and ``EntityRefArrangementOps::addSimpleRelationship()``
methods now accept a boolean indicating whether to bypass
the linear-time check.

The VTK session provides a static method,
``Session::setEnableTranscriptionChecks()``, for operations
to enable/disable this behavior during transcription.

SMTK Project Changes
====================
Changes to smtk::project::ResourceContainer API
-----------------------------------------------

Changes to the ``smtk::project::ResourceContainer`` API to  allow for non-unique roles
to be assigned to Resources in a project.

Deprecated version >= 21.6
~~~~~~~~~~~~~~~~~~~~~~~~~~
``smtk::project::ResourceContainer::getByRole -> smtk::resource::ResourcePtr``

New API
~~~~~~~
``smtk::project::ResourceContainer::findByRole -> std::set<smtk::resource::ResourcePtr>``

Other SMTK Core Changes
=======================
Visitors
--------

SMTK now provides an enumeration, ``smtk::common::Visit``, that visitor lambdas
may return to indicate whether visitation should continue (``smtk::common::Visit::Continue``)
or stop (``smtk::common::Visit::Halt``).
This enum is much easier to use than simply returning a ``bool`` as developers
frequently have trouble remembering which value (true or false) corresponds to
which iteration behaviour.

This is currently only used by ``smtk::task::Task::visit()``, but will be
used more widely in the future.

Task subsystem
--------------

SMTK now provides a task subsystem for representing user-actionable tasks in a workflow.
See the `task-system documentation`_ for more information about how to use this subsystem.

.. _task-system documentation: https://smtk.readthedocs.io/en/latest/userguide/task/index.html
Qt UI Changes
=============
Removing Empty Frames in qtGroupItem
------------------------------------
Using GroupItem's hasRelevantChildren method, qtGroupItem will now hide it's frame if there are no children to be displayed.

Filter Advance Attribute Definitions
------------------------------------

Attribute views will now hide any attribute definitions
that have an advance level that is higher than the user's
active advance level.  This enables the Attribute View to hide
itself if all its definitions should be hidden from the user.

Hide disabled attribute resources
---------------------------------

The Attribute Editor panel will now first check to see if an
Attribute Resource is enabled before attempting to display it.
Telling the Attribute Editor panel to display a disabled Attribute
Resource will be the equivalent to telling the panel to display a
nullptr.  The panel will be reset if it was currently display
any widgets.

Improving UI handling of Signal Operations
------------------------------------------
Originally the qtAttributeView class would ignore the Signal Operation since typically it would be the only Qt UI element that would be creating, removing, and changing the Attributes it is displaying.  However, this prevented the UI designer from having AttributeViews that displayed the same information from being used in Selector Views or have different AttributeViews overlap their contents (for example one View could be displaying Fluid Boundary Conditions, while another was displaying all Boundary Conditions)

This change now encodes the address of the View that initiated the change so that we can avoid a View from being updated via a Signal Operation that it itself initiated.

qtAttributeView has now been updated to only ignore Signal Operations that it triggered.

Supporting smtk.extensions.attribute_view.name_read_only in qtAttributeViews
----------------------------------------------------------------------------
You can now indicate that an Attribute's name should not be modified by creating a bool Property on the Attribute called: **smtk.extensions.attribute_view.name_read_only** and setting its value to true.

Observing Operations
--------------------
qtAttributeView will now properly examine modified attributes to see if they have smtk.extensions.attribute_view.name_read_only property or if their names had been changed.

Changes to Polygon Session
==========================
ImportPPG Operation
-------------------

An ``ImportPPG`` operation has been added to the polygon session
for creating model resources from a simple text file input.
The "ppg" (Planar PolyGon) file format is a simple data format
that specifies 2-D geometry as a list of vertex coordinates and
polygon face definitions, with vertices connected implicitly by
straight-line model edges. Documentation is in the "Session: Polygon"
section of the SMTK user guide.

The ``ImportPPG`` operation is provided as a convenience for exploring
CMB's many capabilities as well as for testing, debug, and demonstration.
To use this feature from modelbuilder, the "File -> New Resource" menu
item now includes an option "Polygon -> Planar Polygon Model from PPG
File".

Removed Previously Deprecated API
=================================
The following deprecated methods have been removed:

* Categories::Set::mode has been replaced with Categories::Set::inclusionMode
* Categories::Set::setMode has been replaced with Categories::Set::setInclusionMode
* Categories::Set::categoryNames has been replaced with Categories::Set::includedCategoryNames
* Categories::Set::set has been replaced with Categories::Set::setInclusions
* Categories::Set::insert has been replaced with Categories::Set::insertInclusion
* Categories::Set::erase has been replaced with Categories::Set::eraseInclusion
* Categories::Set::size has been replaced with Categories::Set::inclusionSize
* ReferenceItem::objectValue has been replaced with ReferenceItem::value
* ReferenceItem::setObjectValue has been replaced with ReferenceItem::setValue
* ReferenceItem::appendObjectValue has been replaced with ReferenceItem::appendValue
* ReferenceItem::setObjectValues has been replaced with ReferenceItem::setValues
* ReferenceItem::appendObjectValues has been replaced with ReferenceItem::appendValues
* PhraseModel::addSource now accepts const smtk::common::TypeContainer&
* PhraseModel::removeSource now accepts const smtk::common::TypeContainer&

Software Process Changes
========================
CMake Policies
--------------

Because of CMake policy CMP0115 (source file extensions must be explicit),
when passing test names to the ``smtk_add_test`` macro, be sure to include
the filename extension (such as ``.cxx``).

Deprecate Python 2.x support
----------------------------

Python 2.x reached its end of life in January 2020. SMTK has deprecated its
Python 2 support and will remove it in a future release.

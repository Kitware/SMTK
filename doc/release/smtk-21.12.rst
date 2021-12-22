.. _release-notes-21.12:

=========================
SMTK 21.12 Release Notes
=========================

See also :ref:`release-notes-21.11` for previous changes.

SMTK Common Changes
===================================

Changes to smtk::common::Instances
----------------------------------
* Made the visit method const.

SMTK Attribute Resource Changes
===================================

Added Definition::isRevelvant Method
------------------------------------

This method will return true if the Definition is considered relevant. If includeCategories is true then
the attribute resource's active categories must pass the Definition's category check.
If includeReadAccess is true then the Definition's advance level < the read access level provided.

Expanded attribute's Signal Operation to include category changes
-----------------------------------------------------------------
The Signal operation now includes a resource item called **categoriesModified** both as a parameter and as a result.
If set, it indicates the attribute resources whose categories have been modified.

Change to Attribute::isRelevant Method
--------------------------------------------------
The parameter readAccessLevel has been changed from int to unsigned int to match the signature of Item::isRelevant.

SMTK Qt Changes
===============

qtAttributeView now uses smtk::attribute::Definition::isRelevant method
------------------------------------------------------

* Removed qtAttributeViewInternals::removeAdvancedDefs since this logic is now part of the Definition's isRelevant method.
Similarly, qtAttributeViewInternals::getCurrentDefs has been simplified by using Definition's isRelevant method.

* qtAttributeViewInternals::getCurrentDefs removed attribute resource parameter.  It was no longer needed.

SMTK View Changes
=================

Changes to Descriptive Phrase Functionality
-------------------------------------------

PhraseModel Changes
~~~~~~~~~~~~~~~~~~~

Deprecated Old Style Visitor Signature
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The deprecated signature returned a boolean.  The support form now returns a smtk::common::Visit Enum.

.. code-block:: c++

  // Deprecated Form
    using SourceVisitor = std::function<bool(
      const smtk::resource::ManagerPtr&,
      const smtk::operation::ManagerPtr&,
      const smtk::view::ManagerPtr&,
      const smtk::view::SelectionPtr&)>;

  // New Form
    using SourceVisitorFunction = std::function<smtk::common::Visit(
      const smtk::resource::ManagerPtr&,
      const smtk::operation::ManagerPtr&,
      const smtk::view::ManagerPtr&,
      const smtk::view::SelectionPtr&)>;

Added Map to find Descriptive Phrase by Persistent Object
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
PhraseModel now maintains a map a Persistent Object's UUIDs to a set of
Descriptive Phrases that contains that Persistent Object.  PhraseModel::trigger method now maintains the map
on Persistent Object insertion or removal.

The method PhraseModel::uuidPhraseMap returns the map.

Also added a protected method PhraseModel::removeFromMap to remove Descriptive Phrases from the map.

TriggerDataChanged Emits 1 Trigger
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Previously, this method would result in a trigger per Phrase in the model.  Now it will only trigger for the root of the model.

DescriptivePhrase Changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~
Added a hasChildren method to indicate if the Phrase has / will have sub-phrases.  This is more efficient than requiring the Phrase's sub-phrases to be built in order to determine if the Phrase has children.

ObjectGroupPhraseContent Changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~
Added a hasChildren method to indicate if the Phrase Content has / will have children Descriptive Phrases.  This is more efficient than requiring the Phrase's sub-phrases to be built in order to determine if the Phrase has children.

Subphrase Generator Changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Added hasChildren method to indicate if the generator would create children sub-phrases for a given Descriptive Phrase.
* Added SubphraseGenerator::parentObjects method to return the Descriptive Phrases that would be parents of new phrases created for a Persistent Object.  **Note** due to performance issues with ModelResource::BordantEntities method, this method has some performance issues.
* Added SubphraseGenerator::createSubPhrase method that uses the above method to directly insert a new Phrase instead of doing a tree traversal.  Performance analysis showed that this approach is slower than the original method due to Model Resource's issues, but the approach seems sound so its been left in so that non-ModelResource Phrase Generators could use it.
* API Change

  * SubphraseGenerator::indexOfObjectInParent now takes in  a const smtk::view::DescriptivePhrasePtr& actualParent instead of a  smtk::view::DescriptivePhrasePtr& actualParent

qrDescriptivePhraseModel Changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The class no longer creates sub-phases when trying to determine if a Qt Item should indicate if it has children.  This speeds things up when dealing with large Phrase Models.

VisibilityBadge Changes
^^^^^^^^^^^^^^^^^^^^^^^
* componentVisibilityChanged no longer triggers the Phase Model.  The original trigger was not needed and caused a performance issue on large Phrase Models.

SMTK Task Changes
=================

Changed FillOutAttribute JSON Logic
-----------------------------------

Code now uses find instead of contains. This avoids doing double searches.

Task now supports Instance criteria
-----------------------------------
In addition to providing a list of Definitions names, you can now also specify a list of Attribute Names that also must be valid for the task
to be considered Completable.


Task now processes category changes on resources
-------------------------------------------------
A Signal operation identifying category changes now will trigger the task to recalculate its internal state.

Fixed Task State Calculations
-----------------------------

* If there are no resources found for the task, the state is now Unavailable
* If all of the task's definitions and instances are irrelevant then the task state is Irrelevant
* If any of the attributes required by the task are invalid, then the task is Incomplete
* Else the task is Completable

Added the following methods
---------------------------
* hasRelevantInfomation - returns true if it finds a relevant attribute or definition related to the task.  It will also determine if it found any attribute resources required by the task.

* testValidity - tests to see if an attribute is not currently in a ResourceAttributes entry.  If it is not then it's validity is tested, it is inserted into the appropriate set, and the method returns true, else it returns false.

Added smtk::tasks::Instances::findByTitle method
------------------------------------------------
Returns the tasks with a given title.

Improved TestTaskGroup
----------------------
The test now validates the states of the tasks after each modification.


Python Related Changes
======================

Operation python tracing with config
------------------------------------

Python traces of operations now use a python dictionary to configure
the inputs to the operation. This is much easier to read and edit
than the complete XML used previously, and allows shorthand for
simple items.

Group items are not yet supported, and nested items must currently
be retrieved with their "path" instead of their "name".


Other Changes
=============

GDAL dependency has been removed
--------------------------------

When building SMTK with VTK and/or ParaView support enabled,
we no longer require them to include support for GDAL.
Functionality has not been removed from SMTK, simply put
behind an option that is off by default.

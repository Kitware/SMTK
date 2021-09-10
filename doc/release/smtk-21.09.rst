.. _release-notes-21.09:

=========================
SMTK 21.09 Release Notes
=========================

See also :ref:`release-notes-21.07` for previous changes.


SMTK Resource and Component Changes
===================================

Supporting MarkedForRemoval
--------------------------
Resources can now be markedForRemoval indicating that the resource will be removed from memory (as apposed to deletion which also means it is being deleted from storage as well).  This can be used in the UI to determine if a View needs to worry about keeping its contents up to date if the reason it is using is going to be removed.  This also gets around a current issue with the Resource Links system which will cause a resource to be pulled back into memory even though the resources that its associated with is going to be removed.

Another benefit is to potentially optimize the removal of components when its resource is targeted for removal.

Developer changes
~~~~~~~~~~~~~~~~~~
* Added the following API to smtk::resource::Resource
  * setMarkedForRemoval - sets the resource's marked for removal state
  * isMarkedForRemoval - returns the resource's marked for removal state
* UI class changes
  * All Attribute Related Qt classes that handle operations will now check to see if the resource is marked for removal.


Attribute Resource Changes
==========================

Changing Expression Format
--------------------------
JSON will now store a ValueItem's Expression in ComponentItem format using the key "ExpressionReference" instead of 2 keys called "Expression" and "ExpressionName".  This no only simplifies things format wise but will also support expressions stored in different resources.

**Note** The older format is still supported so this change is backward compatible.
**Note** The XML format is still using the older style.

Supporting "External" Expressions
---------------------------------
There are use cases where the workflow may want to store expressions in a separate Attribute Resource.
The core of SMTK already supported this but the UI system assumed that the Attribute Resource which owned the ValueItem was also the source for expressions.  This is no longer the case.

qtInstancedView can now optionally take in an Attribute Resource instead of solely relying on the one associated with the UI Manager.  This allows classes like the qtAttributeEditor to supply the Attribute Resource.

Added a new query function called: findResourceContainingDefinition that will return an Attribute Resource that contains an Attribute Definition referred to by its typename.  If the optional Attribute Resource provided to the function also contains the Definition, it is returned immediately without doing any additional searching.  This maintains the original use case where the expressions are stored in the same resource.

qtInputItem no longer assumes the Item's Attribute Resource is the one being used as a source for expressions.

Added two template files that can be used to demo the functionality.

* `data/attribute/attribute_collection/externalExpressionsPt1.sbt` - Contains an Attribute Definition with an Item that can use an expression that is not defined in that template

* `data/attribute/attribute_collection/externalExpressionsPt2.sbt` - Contains an Attribute Definition that represents the expressions used in Pt1.


Qt UI Changes
=============

Changes to qtBaseAttributeView
------------------------------
In the past classes derived from qtBaseAttributeView relied on the qtUIManager to get access to the Attribute Resource they depended on.  This is no longer the case.  The Attribute Resource is now part of the information used to defined the instance.

Deprecation of qtUIManager::attResource() method
------------------------------------------------
This method is no longer needed for qtBaseAttributeView derived classes.

Added qtItem::attributeResource() method
----------------------------------------
This is a convenience method for accessing the qtItem's underlying Attribute Resource

Fixing qtAttributeView handling of Operations
---------------------------------------------
In qtAttributeView::handleOperationEvent, the code had the following issues:

1. It assumed that an Attribute that is in the operation's result must be based on the View's current definition.  This is only true if the View only contained one Definition.  In reality, the attribute's Definition needs to be tested against the Definitions that defined the View and if it matches any of them it needs to be processed.
2. It always assumed that there was a Current Attribute selected.  This would result in a crash if there wasn't any.
3. There was a bug in qtAttributeView::getItemFromAttribute, it was getting items from column 0 instead of the name column.  This resulted in names not being properly updated

Added qtAttributeView::matchesDefinitions
-----------------------------------------
This method tests a Definition against the Definitions that define the View and returns true if it matches or is derived from any in the list.

Replacing qtBaseView::getObject()
---------------------------------
This method returns the instance's view::Configuration but the name didn't reflect that.  Also the getObject method returned a smtk::view::ConfigurationPtr which means that a new shared pointer was always being created and as a result, incrementing its reference count.  The new configuration() method returns a const smtk::view::ConfigurationPtr& instead which does not effect the shared point reference count.

Developer changes
~~~~~~~~~~~~~~~~~
* `qtBaseView::getObject()` method is now deprecated.

Added Ability to Set Attribute Editor Panel's Title
----------------------------------------------------
The Attribute Editor Panel name can now be configure by a smtk::view::Configuration.

If the Configuration is Top Level then the following Configuration Attributes can be used:

* AttributePanelTitle - defines the base name of the Panel.  If not specified it defaults to Attribute Editor.
* IncludeResourceNameInPanel - if specified and set to true, the Panel's title will include the name of the resource in ()

SimpleAttribute.sbt contains an example:

.. code-block:: xml

  <Views>
    <View Type="Attribute" Title="External Expression Test Pt - Source" TopLevel="true" DisableTopButtons="false"
      AttributePanelTitle="SMTK Test" IncludeResourceNameInPanel="t">
      <AttributeTypes>
        <Att Type="B-expressions"/>
      </AttributeTypes>
    </View>
  </Views>

Developer changes
~~~~~~~~~~~~~~~~~~
* `pqSMTKAttributePanel::updateTitle()` now takes in a `const smtk::view::ConfigurationPtr&` argument.


SMTK Task Subsystem (Preview)
=============================

New Task Classes
----------------
The task subsystem now provides more task types, task-adaptor classes
for configuring tasks as they change state, and additional tests.
See the `task class documentation`_ for details.

Tasks now include "style" strings that will be used to configure
application state when the task becomes active.

Tasks now include references to dependencies and dependents,
children and a parent. These are used to provide workflow
observers that user interfaces can use to monitor when tasks
are added-to and removed-from a pipeline.

.. _task class documentation: https://smtk.readthedocs.io/en/latest/userguide/task/classes.html

Task serialization/deserialization
----------------------------------
The task classes and task manager now support serialization
and deserialization (to/from JSON). See the TestTaskJSON
test and user guide for more details.


Other SMTK Core Changes
=======================

Using TypeContainers instead of ViewInfo
----------------------------------------

In order to make the View System more flexible and to work with the new Task System, the following changes were made:

* smtk::view::Information is now derived from TypeContainer and is no longer an abstract class.  As a result it can now do the job that ViewInfo and OperationViewInfo does
* ViewInfo and OperationViewInfo are no longer needed.
* qtBaseView's m_viewInfo is now an instance of smtk::view::Information and not ViewInfo

Developer changes
~~~~~~~~~~~~~~~~~~

Unless the qtView is directly accessing m_viewInfo, there should be no required changes.

When dealing with smtk::view::information, it is important that the type you insert into the container exactly matches the type you use to get information from the container.  For example if you insert a QPushButton* into the container and attempt to get a QWidget* back, it will fail and throw an exception.

So it is recommended you explicitly state the template type instead of having the compiler determine it. In the above example you would need to do an insert<QWidget*>(myQtPushButton) in order to get a QWidget* back.

Removed Data Structures
+++++++++++++++++++++++
smtk::external::ViewInfo and smtk::external::OperatorViewInfo are no longer needed and have been removed.  smtk::view::Information object should be used instead.


Visibility badge improvements
-----------------------------
The ParaView visibility-badge extension had an issue when large numbers
of phrase-model instances existed and a resource was closed: the visibility
was updated by completely rebuilding the map of visible entities which
is slow. This is now fixed.

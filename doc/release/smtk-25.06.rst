.. _release-notes-25.06:

=========================
SMTK 25.06 Release Notes
=========================

See also :ref:`release-notes-24.11` for previous changes.


Changes to SMTK Resources
=========================

Unit System API Renaming
------------------------

To improve clarity and align with common engineering terminology, several methods related to unit systems in SMTK's `Resource` and `Attribute` classes have been renamed. Specifically, the word "units" has been changed to "unit" to reflect that the API refers to a single system that encompasses multiple units.

Updated Method Names in `smtk::resource::Resource`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- ``unitsSystem()`` → :smtk:`unitSystem()<smtk::resource::Resource::unitSystem()>`
- ``setUnitsSystem()`` → :smtk:`setUnitSystem()<smtk::resource::Resource::setUnitSystem()>`

Updated Method Names in `smtk::attribute`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- :smtk:`Definition::setItemDefinitionUnitsSystem()<smtk::attribute::Definition::setItemDefinitionUnitsSystem()>` →
  :smtk:`Definition::setItemDefinitionUnitSystem()<smtk::attribute::Definition::setItemDefinitionUnitSystem()>`

- :smtk:`ItemDefinition::setUnitsSystem()<smtk::attribute::ItemDefinition::setUnitsSystem()>` →
  :smtk:`ItemDefinition::setUnitSystem()<smtk::attribute::ItemDefinition::setUnitSystem()>`

These renamings are intended to enhance consistency and improve the intuitiveness of the API. Users updating existing code will need to apply these name changes accordingly.

SMTK Attribute Resource Enhancements
=====================================

The following updates have been made to improve the functionality, performance, and extensibility of SMTK’s Attribute subsystem:

Geometry Update Support via Signal Operations
---------------------------------------------

Renderable geometry for attribute resources and their components is now better supported:

* The `smtk::attribute::Signal` operation has been enhanced to properly mark created, modified, and expunged components for geometry updates—**but only if** the attribute resource has a registered geometry provider.
* The `pqSMTKResource` class now includes Signal operations when updating visual representations of attribute resources, provided the resource supplies a geometry object.

These changes remove several obstacles to rendering attribute-based geometries in views like ParaView.

Category Functionality Moved to smtk::common
---------------------------------------------

All category-related functionality has been **migrated from `smtk::attribute` to `smtk::common`**. This change enables broader use of categories across SMTK subsystems such as:

* Task Managers
* Task Worklets
* Tasks

 ⚠️ **Namespace Change Required:**
   Any use of `smtk::attribute::Categories` must be updated to `smtk::common::Categories`.
   Similarly, category-based expression grammars have also been relocated and require the same namespace update.
   These changes are straightforward and typically involve simple text substitutions in your codebase.



Improved Reference Item Iteration Support
-----------------------------------------

The `ComponentItem` class now provides full iterator support:

* `ComponentItem` now offers a standard `const_iterator` API.
* This enables the use of `ComponentItem` in C++11 range-based for-loops and compatibility with standard library algorithms.

This improvement makes it easier to write clean, modern C++ code that iterates over referenced components.

Custom Validity Functions for Items
-----------------------------------

You can now define **custom validity logic** for individual `Item` objects:

* The `Item::isValid()` method now delegates to either a user-defined function (if set) or a new `Item::defaultIsValid()` method.
* Use `Item::setCustomIsValid()` to register your custom validation function.

 📌 **Example Usage:**
 Refer to `smtk/attribute/testing/cxx/customIsValidTest.cxx` for a working example.

This feature is useful for modeling domain-specific constraints or dynamic validation requirements.

Python API Enhancements for Value Items
---------------------------------------

To better support multi-value items in Python, the following pybind11-wrapped item types now include convenient array-based methods:

* `DoubleItem`
* `IntItem`
* `StringItem` (if applicable)

New Methods:
~~~~~~~~~~~~

* `setValues(values: List[T])` – Set all item values from a list or array.
* `values() -> List[T]` – Retrieve all item values as a list.

These methods replace C++ iterator-based APIs that are not easily wrapped in Python, making the interface more Pythonic and user-friendly.

SMTK Geometry Subsystem Enhancements
===================================

Improved Support for Attribute Resources with Renderable Geometry
-----------------------------------------------------------------

The ``smtk::extension::vtk::source::SourceFromAttribute`` class has been **removed**.

This class was originally used to ensure that attribute resources would generate a ParaView pipeline object. However, it did **not** provide any renderable geometry and often conflicted with other plugins designed to render attribute resources properly.

With recent improvements to SMTK's geometry handling and ParaView integration, the need for this class has been eliminated. Attribute resources with registered geometry are now rendered correctly without requiring this placeholder source, resulting in cleaner behavior and improved plugin interoperability.

SMTK Operation System Enhancements
==================================

Per-Instance Operation Handlers
-------------------------------

SMTK operations now support **per-instance handlers**, allowing developers to register callback functions that will be invoked upon the completion of a specific operation instance—regardless of whether the result indicates success or failure.

Handlers differ from observers in the following ways:

- **Scope**: Handlers apply only to the specific instance of an operation to which they are attached. Multiple instances of the same operation type must each have their own handlers.
- **Invocation Timing**: Handlers are called exactly once (or not at all) per operation execution. After invocation, they are automatically cleared.
- **Thread Context**: Handlers run on the **same thread as the operation**, unlike observers, which may run on the main/GUI thread in Qt applications.
- **Signature**: Handlers receive only the operation and its result (no `EventType`) and cannot cancel the operation.

This feature is useful for workflows that require custom post-processing or logic tightly coupled to a single operation's result. **Note** that it is safe for a handler to re-register itself during invocation.

Render Visibility Hint Support
------------------------------

Operations can now embed **render visibility hints** in their results to suggest visibility settings for components and resources in the application UI.

This functionality is especially useful for `smtk::task::EmplaceWorklet` subclasses that create geometric resources which should remain hidden until their associated tasks become active. Applications can use this hint to dynamically adjust the visibility of rendered objects based on task context.

Expanded Access to Operation Members
------------------------------------

Several member functions of :smtk:`smtk::operation::Operation` have been promoted from `private` or `protected` to `protected` or `public` visibility. This change improves Python interoperability and allows Python-based operations to **invoke nested operations**, enabling more flexible and composable workflows from Python scripts.


SMTK Tasks Enhancements
=====================

Supporting Categories on Worklets
---------------------------------

Worklets now support categories, offering an additional layer of conceptualization. This functionality is similar to how categories are used within the attribute resource. The addition of categories allows both Tasks and the Task Manager to use this information to determine if tasks generated by a worklet are appropriate as child tasks or top-level tasks within a workflow.

For an example, see ``data/projects/SimpleWorkletExample/workletsWithCategories.smtk``.

API Changes to ``smtk::task::Worklet``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- ``setCategories(...)``: Assigns a set of category strings to a worklet.
- ``categories()``: Returns the set of category strings assigned to the worklet.

Adding Category Expression Support to Task Manager
--------------------------------------------------

The Task Manager now supports associating a Category Expression, which determines if tasks created by a worklet are suitable as top-level tasks based on the worklet's categories. The category expression is defined in the manager's configuration.

 **Note:** Category evaluation for determining whether a task will accept a worklet as a child is only applied by the user interface to limit the options presented to users. This constraint is not enforced by the core SMTK library within task classes or the ``EmplaceWorklet`` operation.

API Changes to ``smtk::task::Manager``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- ``toplevelExpression()``: Returns a reference to a ``smtk::common::Categories::Expression``.

Adding Category Constraints to Tasks
------------------------------------

Tasks can now indicate whether the tasks generated by a worklet are appropriate as child tasks based on their associated categories. This functionality is enabled by the new ``acceptsChildCategories`` method in the ``task::Agent`` class, which evaluates the categories and returns one of the following results:

- ``CategoryEvaluation::Pass`` – the agent determines the categories are acceptable for a child task.
- ``CategoryEvaluation::Reject`` – the agent determines the categories are not appropriate for a child task.
- ``CategoryEvaluation::Neutral`` – the agent cannot determine the appropriateness of the categories.

When making this determination, the task asks all of its agents to evaluate the categories. If any agent rejects the categories, the task will not accept them as appropriate children. If at least one agent accepts the categories, the task will accept them. If all agents are neutral, the task will reject them.

.. note::

   Category evaluation is employed by the user interface to constrain available options, but it is not enforced by the core SMTK library in task classes or the ``EmplaceWorklet`` operation.

API Changes
~~~~~~~~~~~

``smtk::task::Task``
++++++++++++++++++++

- ``acceptsChildCategories(...)``: Returns ``true`` if none of its agents reject the categories and at least one agent passes them.

``smtk::task::Agent``
+++++++++++++++++++++

- ``acceptsChildCategories(...)``: Returns the agent's evaluation of the categories. The base class will return ``CategoryEvaluation::Neutral``.

Adding ``ChildCategoriesAgent``
-------------------------------

A new agent class, :smtk:`ChildCategoriesAgent <smtk::task::ChildCategoriesAgent>`, has been introduced to provide category expression support for a task. This agent uses a ``smtk::common::Categories::Expression`` for category evaluation. By default, the expression is set to reject all categories.

Active Task Notifications
-------------------------

Observers of the :smtk:`smtk::task::Active` object are now invoked after the active task has been changed. This ensures that responders fetch the active task inside the observer, which will reflect the "next" task rather than the current one.

Task Diagnostics
----------------

Tasks and agents now collaborate to provide description and diagnostic information for presentation to users. The :smtk:`task <smtk::task::Task>` class can now include an XHTML description to present workflow designer information to users. This description is combined with diagnostic XHTML produced by the :smtk:`Agent <smtk::task::Agent::troubleshoot>` method.

The amount of diagnostic information currently available from SMTK's agents is limited but is expected to expand over time.

Agent for Producing Fixed Port Data
-----------------------------------

SMTK now includes a :smtk:`TrivialProducerAgent <smtk::task::TrivialProducerAgent>` that can be configured to produce fixed data (in the form of :smtk:`smtk::task::ObjectsInRoles`) on an output port.

This agent is intended to be used by subclasses of :smtk:`smtk::task::EmplaceWorklet` that add new resources to projects along with the tasks of the worklet. These resources (and/or components) can be configured to appear on a top-level task's output port using this agent, which simply returns an internal ``ObjectsInRoles`` object when requested for port data.

Handle Unset Values
-------------------

Previously, the worklets panel would fail to fully populate if an operation initializing the task system returned an unset value. Now, a warning message is logged, and normal insertion of worklets continues.

SMTK ParaView Extensions Enhancements
=====================================

New Operation Toolbar for Applications
--------------------------------------

SMTK now includes the new :smtk:`pqSMTKOperationToolbar` class, designed to simplify the integration of SMTK operations into ParaView-based applications. Developers can inherit from this class to easily add a customizable toolbar featuring buttons that launch SMTK operations.

The toolbar behavior adapts to the operation's parameter requirements:

* **Immediate Execution** – Operations with default-valid parameters are executed immediately upon button click.
* **Interactive Editing** – Operations that require user-defined parameters will automatically raise the operation editor panel, allowing users to configure inputs before execution.

This enhancement streamlines user workflows by providing quick access to commonly used operations while ensuring flexibility when parameter input is required.

SMTK Qt Extensions Enhancements
===================================

This release introduces several significant improvements and new capabilities to the Qt Extensions in SMTK, focusing on enhanced task editing, UI flexibility, and expression filtering. Below is a summary of the major updates.

Expression UI Enhancements
--------------------------

**Improved Expression Selection in** ``qtInputsItem``

To support the growing complexity of expressions in ``ValueItem``\ s, the ``qtInputsItem`` widget now includes an optional filtering mechanism:

- A new QPushButton toggles visibility of a QLineEdit, which allows users to enter a regular expression to filter entries in the expression QComboBox.
- Filtering is implemented using a custom QSortFilterProxyModel, ensuring that the current expression and core options like "Please Select" or "Create..." remain visible.
- Each QComboBox item now displays a tooltip indicating the type of the corresponding expression.

Redesigned Task Editor Components
---------------------------------

**New** ``qtTaskNode`` **Implementation**

A fully redesigned ``qtTaskNode`` class now leverages ``QGraphicsItem`` exclusively (no Qt Widgets), offering a cleaner UI and resolving Mac-specific transformation issues.

The new design comprises:

- A color-coded state indicator
- A label item managing task names and activation
- A visual toggle for marking tasks as completed

Task Diagram Editor Improvements
--------------------------------

**Port Snapping and Connectivity**

The ``qtTaskEditor`` now supports improved layout behavior and clearer connectivity:

*Port Snapping*:

- Ports can now automatically align near their task node using:
  - ``snapPortsToTask()`` / ``setSnapPortsToTask(bool)``
  - ``snapPortOffset()`` / ``setSnapPortOffset(int)``

*Port Connection Curves*:

- Visual curves now connect external port nodes to task nodes:
  - Controlled via ``drawPortsToTaskCurves()`` and ``setDrawPortsToTaskCurves(bool)``

**Color Configuration Consolidation**

New centralized control over UI color themes through ``qtDiagramViewConfiguration``:

- Supports both light and dark palettes
- Provides customizable methods for node, port, text, border, and background colors
- New method ``colorFromToken(token)`` replaces the deprecated ``colorForArcType``

**Node Layout Tools in Diagram Panel**

The ``qtDiagram`` panel toolbar now includes:

- Viewport controls (zoom to fit all or selected nodes)
- Node alignment, distribution, and layout tools
- New ``tools()`` method enables diagram modes to inject custom toolbar actions

Task Path Navigation and UI for Children
----------------------------------------

**Task Path Support and Visualization**

Enhancements have been made to navigate and display child tasks within ``qtTaskEditor``:

- A task "breadcrumb" path appears above the diagram for navigation
- When activating a task with children:
  - The view displays only child tasks and relevant ports/arcs
  - Users can "zoom out" via the breadcrumb interface

**Displaying Child Status in** ``qtTaskNode``

Visual indicators using Font Awesome Unicode characters now show whether a task:

- Has child tasks (solid folder icon)
- Can accept child tasks via worklets (outlined folder icon)

To support this:

- ``canAcceptWorklets()`` has moved to ``smtk::task::Task``
- Font Awesome 6 Free (regular and bold) fonts are now included

Task Worklet Category Filtering
-------------------------------

Enhancements to support category-aware task worklets:

**qtTaskPath**

- Now includes tasks that can accept worklets (even if they have no current children)
- Uses ``canAcceptWorklets()`` to determine eligibility

**qtWorkletModel and qtWorkletPalette**

- Both now accept a ``smtk::common::Categories::Expression`` to filter worklets based on category constraints
- Accept a parent ``Task`` to determine context for valid worklets
- A ``rebuild()`` method has been added to update the model when the parent task changes

**qtTaskEditor**

- Automatically updates the worklet palette if the active task changes via an operation
- If no task is active, the palette uses the current task path tail as context
- Ensures worklets create children of the designated parent task

**Operation Update**

- ``EmplaceWorklet`` operation now supports an optional ``parentTask`` parameter to assign parent-child relationships to new tasks

Task Editor Usability Fixes
---------------------------

**Task Tooltips**

- Tooltips now leverage task and agent descriptions for richer UI feedback in the diagram

**Arc Visibility Fixes**

- Diagrams now correctly update the visibility of port arcs during construction and task switching

Signal and Event Changes in qtItem
----------------------------------

**Modified Signal Update**

- ``qtItem::modified`` now takes a pointer to the modified item as an argument:

  .. code-block:: cpp

     Q_EMIT this->modified(this);

- ``qtItem::childModified`` has been **removed**. Use the new ``modified(qtItem*)`` signal instead.

Developer-Level Changes
-----------------------

- All ``qtBaseNode`` classes now inherit from ``QGraphicsObject`` (no longer require both ``QObject`` and ``QGraphicsItem``)
- Creating Projects no longer requires manually setting defaults (``resources``, ``operations``, etc.)
- Project Read operations now rely on the base resource read mechanism for ID and file location
- ``checkDependencies`` now also checks child tasks for cycle detection and enforces same-parent validation for connections
- Tasks now support dynamic addition and removal of children, with cycle prevention
- JSON serialization now correctly handles task hierarchies
- The :smtk:`qtOperationTypeModel` class has been extended to insert buttons into a provided toolbar. This is used by the :smtk:`pqSMTKOperationToolbar` class to insert toolbar buttons using the active server's operation-model.

 **Note:** These updates collectively provide improved UI functionality, increased extensibility for task workflows, and more robust integration of expression and worklet systems.

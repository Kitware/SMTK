.. _smtk-pv-panels:

Panels
------

SMTK provides several panels for use in ParaView-based applications

* a resource tree panel that displays a configurable hierarchy of resources and their components.
* an attribute panel that displays and edits attribute resources.
* a "legacy" operation panel that displays and edits operation parameters.
* an operation toolbox panel that displays operations.
* an operation parameter-editor panel that lets users edit operation parameters before running them.

The last two operation panels are separate to allow users with small displays
(laptops, tablets, etc.) arrange them separately for better use of available space.

Most panels register themselves with ParaView by calling ``pqApplicationCore::registerManager()``
and can thus be retrieved by calling ``pqApplicationCore::manager()`` with their registered
name (listed in the sections below).

Panels may also register themselves as :smtk:`smtk::view::UIElementState` so that operations
(especially those that read and write projects) can serialize/deserialize their state.
The diagram panel in particular is an example of this.

.. _smtk-pv-resource-panel:

Resource panel
==============

The :smtk:`pqSMTKResourcePanel` exposes an instance of :smtk:`pqSMTKResourceBrowser`
to show a tree-view of resources and components.
It is registered with the pqApplicationCore instance as a manager named "smtk resource panel".
The panel has a ``setView()`` method that accepts a view configuration
so your application can reconfigure the panel.

The default panel configuration is in ``smtk/extension/qt/ResourcePanelConfiguration.json``,
but if your application includes a plugin that provides an
:smtk:`smtk::paraview::ApplicationConfiguration` object, you may override it at startup.
Plugins can also change the panel configuration at any time while your application is running.

The resource panel configuration accepts settings for
+ a phrase model, which determines what appears at the top level of the resource-panel's tree;
+ a subphrase generator, which determines which children appear beneath the top-level items;
+ and a set of badges that may appear next to phrases in the tree.

Added Ternary Visibility Badge Support)

.. _smtk-pv-attribute-panel:

Attribute panel
===============

The :smtk:`pqSMTKAttributePanel` displays the top-level view of an SMTK attribute resource.
It is registered with the pqApplicationCore instance as a manager named "smtk attribute panel".
It can also be programmatically configured to show views (including non-top-level views) by
calling the ``displayResource()`` or ``displayView()`` methods.

When a task-based workflow is used (i.e., a document with a task manager is loaded), the
active task's style tags can dictate a view that the attribute panel should display.
When using the task-manager's style to determine what to display, the attribute panel will
call the active task's :smtk:`smtk::task::Task::getViewData` method to obtain relevant
attribute resources; these resources will be queried to find a view-configuration whose name
matches the one specified by the style.

.. _smtk-pv-legacy-operation-panel:

Legacy operation view
=====================

The :smtk:`pqSMTKOperationPanel` was created mainly for debugging operations and not
intended for production use (although it has been used as such).
It is registered with the pqApplicationCore instance as a manager named "smtk operation panel".
It contains a vertical splitter with a list of operations at top (limited by default
to those operations that match the current SMTK selection);
an operation view (either a :smtk:`qtOperationView <smtk::extension::qtOperationView>`
or a custom :smtk:`qtBaseView <smtk::extension::qtBaseView>`) beneath that for
editing parameters;
and an a widget at the bottom that was intended to show documentation but is largely unused.
At some point in the future, this panel will likely be deprecated and removed.
You should prefer the operation toolbox and parameter-editor panels below as replacements.

.. _smtk-pv-operation-toolbox-panel:

Operation toolbox panel
=======================

The :smtk:`pqSMTKOperationToolboxPanel` provides a searchable list of operations.
It is registered with the pqApplicationCore instance as a manager named "smtk operation toolbox".
The list can be filtered

+ by search text input from the user;
+ by a whitelist of operations (passed to the view-widget's model); and
+ by how applicable the current SMTK selection is to the operation's associations.

Operations appear as a grid of push-buttons with icons and the text of the
operation's label.
If an operation can be run without editing any parameters (other than associations,
which are configured using the SMTK selection), then clicking a push-button will
run the operation immediately.
Long-clicking a push-button will emit a signal that the parameter-editor panel
below accepts to allow further user configuration before running.

If the toolbox is configured to allow searching, you can activate the search bar
for operations (i.e., switch the keyboard focus to the search bar) at any time by
pressing ``Ctrl+Space`` (or ``Cmd+Space`` on macos). While the search bar has focus,
pressing the ``Return`` key will emit a signal to edit the parameters of the first
(top, left-most) push button in the grid.

If the toolbox is configured to allow it, all operations (not just those available
to the currently-selection objects) will be displayed without decoration and may
be filtered by searching.

.. _smtk-pv-parameter-editor-panel:

Operation parameter-editor panel
================================

The :smtk:`pqSMTKOperationParameterPanel` provides a tab-widget holding
zero or more operation or custom views (a combination of
:smtk:`qtOperationView <smtk::extension::qtOperationView>` or
:smtk:`qtBaseView <smtk::extension::qtBaseView>` instances).
It is registered with the pqApplicationCore instance as a manager named "smtk operation parameters".
Upon creation, it attempts to connect to signals provided by the operation toolbox
described above. If the toolbox panel exists, the parameter panel will respond
to its requests for editing.
Otherwise, your application must configure it to edit operations directly.
Similarly, if the toolbox panel exists, the parameter editor will connect operation views to
launch operations via that panel.
Otherwise, your application is responsible for listening for the ``runOperation()`` signal
and launching the provided operation.

Task system support
^^^^^^^^^^^^^^^^^^^^

The :smtk:`pqSMTKOperationParameterPanel` supports projects with task-based workflows; when a new
task becomes active, if it is a :smtk:`SubmitOperation <smtk::task::SubmitOperation>` task, and
any of its style tags contain an ``operation-panel`` section, the panel will update its behavior
based on the following keys:

* ``hide-items``: an array of strings specifying paths to operation parameters to be hidden.
  This key is typically used to hide parameters that are configured by a
  ConfigureOperation adaptor.

Future keys in the ``operation-panel`` section include:

* (future) ``view``: one of the following enumerants specifying where the operation's view configuration
  should come from:

  * ``anew``: the task should create a new view configuration ab initio (i.e., ignoring any
    view configuration provided by the operation itself).
  * ``override``: the task should start with the view provided by the operation itself and
    add item-view configurations for any parameters listed below.
  * ``unmodified``: use the default view provided for the operation's parameters without any
    changes that take the task configuration into consideration.

  If no value is provided, then the view defaults to ``override``.

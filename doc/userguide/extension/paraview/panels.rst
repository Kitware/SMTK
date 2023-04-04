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

Resource panel
==============

The :smtk:`pqSMTKResourcePanel` exposes an instance of :smtk:`pqSMTKResourceBrowser`
to show a tree-view of resources and components.
It is registered with the pqApplicationCore instance as a manager named "smtk resource panel".
The panel has a ``setView()`` method that accepts a view configuration
so your application can reconfigure the panel.

Attribute panel
===============

The :smtk:`pqSMTKAttributePanel` displays the top-level view of an SMTK attribute resource.
It is registered with the pqApplicationCore instance as a manager named "smtk attribute panel".
It can also be programmatically configured to show views (including non-top-level views) by
calling the ``displayResource()`` or ``displayView()`` methods.

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

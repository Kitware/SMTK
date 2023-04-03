.. _release-notes-23.04:

=========================
SMTK 23.04 Release Notes
=========================

See also :ref:`release-notes-23.01` for previous changes.


SMTK Common Related Changes
=====================================

Common Directory Utility
------------------------

Previously, the :smtk:`smtk::common::Paths::directory` method would
throw a boost filesystem exception if a user called it without
permission to the path passed to it. This has been fixed by capturing
the exception internally.

Expanding String token API
--------------------------

String tokens now have additional API:

+ :smtk:`Token::hasValue() <smtk::string::Token::hasValue>`
  returns true the string manager contains a string for the token's integer hash.
  Note that tokens constructed at compiled-time via the string-literal operator
  will not insert the source string into the manager.
+ :smtk:`Token::valid() <smtk::string::Token::valid>`
  returns true if the token has a valid value.
  The string manager reserves a special value (``smtk::string::Manager::Invalid``)
  to indicate an uninitialized or unavailable hash.


SMTK UI Related Changes
=======================

Adding ID Support to Instanced Views
------------------------------------

You can now refer to an Attribute in an Instanced View by its **ID**.  This will allow the View to refer to
the Attribute even if its Name is changed later on.

The View will add the **ID** View Configuration information if it doesn't already exist and will use it
in the future to find the Attribute.

If the **ID** is not specified, the view will continue require the **Name** configuration view attribute to be specified (along with the **Type** configuration view attribute if the named Attribute does not currently exists).

Allow qtOperationDialog to be non-modal
---------------------------------------

The qtOperationDialog can now be shown non-modal, using `show()` instead of `exec()`.  When used as a non-model dialog,
the Apply button doesn't close the dialog, but instead will remain active and
allow the operation to be run again.


Changes to SMTK's Task Subsystem
================================

User Interface for Task-based Workflows (Preview)
-------------------------------------------------

SMTK now provides a ParaView plugin which adds a :smtk:`task panel <pqSMTKTaskPanel>`
similar to ParaView's node editor. Each task is shown as a node in a graph and
may be made active, marked complete (or incomplete), and manually placed by the user.
Each task may have a collection of style keywords associated with it and the
task manager holds settings for these keywords that affect the application state
when matching tasks are made active.

In particular, SMTK's attribute-editor panel can be directed to display any view
held by attribute resource when a related :smtk:`smtk::task::FillOutAttributes` task
becomes active.

This functionality is a preview and still under development; you should expect
changes to user-interface classes that may break backwards compatibility while
this development takes place.

Operation Hint for Switching the Active Task
--------------------------------------------

Any operation can now request the active task be switched
by providing a hint on its result attribute.
Use the ``smtk::operation::addActivateTaskHint()`` function
to add the hint before your operation completes.
Then, the :smtk:`pqSMTKOperationHintsBehavior` object will
observe when the operation has completed and process the
hint and attempt to switch to the matching task.
See ``smtk::project::Read`` for an example.

Task Management Changes
------------------------

The task manager is no longer considered part of an application's state.
Instead of expecting an application's :smtk:`Managers <smtk::common::Managers>` instance to
hold a single task manager, each :smtk:`Project <smtk::project::Project>` owns its own
task manager.

As part of this change, the project read and write operations now include a serialization of
the project's task manager. This means that the task system JSON state is now properly
serialized.

Another part of this change removes the :smtk:`smtk::task::json::jsonManager` structure
with its ``serialize()`` and ``deserialize()`` methods. You should replace calls to
these methods with calls to ``to_json()`` and ``from_json()``, respectively.
Furthermore, you are responsible for pushing an instance of the
:smtk:`task helper <smtk::task::json::Helper>` before these calls and popping the instance
afterward.
See the task tests for examples of this.

Finally, because :smtk:`smtk::common::Managers` no longer contains an application-wide
instance of a :smtk:`smtk::task::Manager`, the signature for :smtk:`Task <smtk::task::Task>`
constructors is changed to additionally accept a parent task manager.
The old signatures will generate compile- and run-time warnings.
The constructors still accept a :smtk:`smtk::common::Managers` since tasks may wish
to monitor the application to determine their state.

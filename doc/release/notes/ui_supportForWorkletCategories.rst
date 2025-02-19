Changes in UI
=============

UI Changes to Support Worklet with Categories
---------------------------------------------

The following classes have been updated to support Task Worklets with Categories.
Please see data/projects/SimpleWorkletExample/workletsWithCategories.smtk as an example.

qtTaskPath
~~~~~~~~~~

* Added a new method called *canAcceptWorklets* which returns true if there is at least one worklet that would generate
tasks that could be accepted as children.

* Previously, the path would only add an active task to its path if had children.  Now it will also add the task if it can accept any worklet.

**Note**: with these changes, a task that has no children but can accept those generated from a worklet will now be added to the path and the diagram will now show nothing.

qtWorkletModel
~~~~~~~~~~~~~~

* Added a method to set a :smtk:`Categories::Expression <smtk::common::Categories::Expression>` that can be used to determine if worklets can be applied to the top-level of the workflow.
* Added a method to set task to be the parent for tasks created by workets
* Added a method to rebuild the model

The first two methods are now used to filter the worklets so that only *acceptable* worklets (based on category constraints and the set parent task, or the top-level expression there is no parent task).  The third method is used when the parent task is changed which may cause the set of displayed worklets to change.

qtWorkletPalette
~~~~~~~~~~~~~~~~

* Added a method to set a :smtk:`Categories::Expression <smtk::common::Categories::Expression>` that can be used to determine if worklets can be applied to the top-level of the workflow.
* Added a method to set task to be the parent for tasks created by workets

These methods are used to set the appropriate methods of its internal qtWorkletModel.

qtTaskEditor
~~~~~~~~~~~~

* If the active task is modified via an operation, the editor will force the worklet palette to be updated.
* If a task no longer active and there is no new active task, the editor will now check to see if task path is currently not empty and if so it will set the tail task to be the worklet palette's parent task.
* If there is a parent task, the editor will set the appropriate parameter in the worklet emplace operation which will now add the new tasks as children to the specified parent.

Other Changes
~~~~~~~~~~~~~
* EmplaceWorklet Operation now has an optional *parentTask* parameter that when set will be used as the parent for any tasks that are created.

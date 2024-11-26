User Interface Changes
======================

Providing UI Support For Task Children
--------------------------------------

Changes have been made to support visualizing the children of a Task
in a qtDiagram.  These changes include:

* The diagram now includes an empty "top" widget that generators can use to display data above the diagram.
* The task editor now displays a "task path" above the diagram; when a task with children is made active, the task path shows a "breadcrumb" with that task and all its ancestors while the diagram shows only children of that task. Users can click the task-path's "home" button or any task in the task path to "zoom out" to show that task's children instead of children of the currently selected task.

* Making a Task with children Tasks active will now cause hide all tasks, ports, and arcs that were originally displayed and instead display the following:
  * The children Tasks and their external Ports
  * The Active Task's internal Ports
  * All Arcs connected to the children Tasks' external Ports
  * All Arcs connected to the Active Task's internal Ports
* Created a qtTaskPath class that can be used to navigate up the current Task Path


Other Changes
~~~~~~~~~~~~~

* It is no longer necessary to specify the following information when manually creating Projects when they are the defaults:
  * resources
  * operations
  * task_manager
  * conceptual_version
* Project Read Operation will no longer explicitly find/set the following information since the base Resource Read mechanism already does it:
  * Id
  * file location
* checkDependencies will now check a Task's children as well to make sure cycles are not created.
* checkDependencies will now check to make sure the two Tasks are either both top-level or owned by the same parent.
* Added the ability to add and remove children of a Task - note that in the case of adding children, the method will make sure cycles are not created.
* JSON serialization/de-serialization will now properly deal with children.

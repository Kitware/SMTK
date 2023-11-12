Introducing Worklets and Galleries to the Task Manager
------------------------------------------------------

There are times when a user will need to interactively extend a task workflow  by adding a tasks or a group of related tasks.  To provide this functionality, SMTK provide the concept of a :smtk:smtk::task::Worklet.  A worklet is defined as an object representing a template for a set of tasks that can be instantiated to reuse some portion of a workflow. In SMTK, a worklet is a subclass of :smtk:smtk::resource::Component.

To manager the worklets, a Gallery class has been added called smtk::task::Gallery and is held by a project's :smtk:smtk::task::Manager.

Developer changes
~~~~~~~~~~~~~~~~~~

* Added the Worklet class and all related JSON and Pybind support
* Added the Gallery class and its Pybind support (it does not need any special JSON support)
* Extended Task Manager to have the following methods:
  * smtk::task::Gallery& gallery()  - to return a gallery of worklets
  * const smtk::task::Gallery& gallery() const - to return a const gallery of worklets

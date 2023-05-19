Task System
-----------

Tasks are now components
~~~~~~~~~~~~~~~~~~~~~~~~

The :smtk:`smtk::task::Task` class now inherits :smtk:`smtk::resource::Component`.
Instances of tasks are still owned by the task manager, but it is now assumed that
the task manager is owned by a :smtk:`Resource <smtk::resource::Resource>`.
This has far-reaching consequences:

+ Tasks may have properties and links (such as associations to attributes).
+ Tasks must not be modified outside of operations (for thread-safety).
+ The parent resource must provide operations to find, insert, and remove tasks.
  The Project class now provides these.

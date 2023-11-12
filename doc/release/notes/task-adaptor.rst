Task System
-----------

Adaptors are now components
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The :smtk:`smtk::task::Adaptor` class now inherits :smtk:`smtk::resource::Component`.
Instances of adaptors are still owned by the task manager, but it is now assumed that
the task manager is owned by a :smtk:`Resource <smtk::resource::Resource>`.
This has far-reaching consequences:

+ Adaptors may have properties and links (such as associations to attributes).
  Note that links are **not** used to model connections from the adaptor to
  its upstream and downstream task.
+ Adaptors must not be modified outside of operations (for thread-safety).
+ The parent resource must provide methods/operations to find, insert, and remove adaptors.
  The Project class now provide methods to find and filter adaptors;
  it provides a read operator that may create adaptors.
  The task system also provides an operator (:smtk:`smtk::task::EmplaceWorklet`)
  that may create adaptors.

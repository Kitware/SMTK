## Resource Manager

Instances of `smtk::resource::Manager` no longer provide direct access to the `smtk::resource::Container`
(a boost multi-index container) in order to provide thread-safe insertion, traversal, and removal of
resources.

Instead of calling `smtk::resource::Manager::resources()`, you should now use one of the following methods:

+ One of the existing `find()` methods to obtain resources by location, or type-index.
+ One of the existing `get()` methods to obtain resources by ID.
+ `void smtk::resource::Manager::visit(const Visitor& visitor) const` — to iterate over all resources
  held by the manager. Note that access to the resource manager from inside the visitor is limited to
  read-only methods. Methods that modify the set of resources held by the manager may not be called until
  after `visit()` exits. The proper strategy for inserting or removing resources is to have the visitor
  capture a list of changes queued during visitation and perform modifications once visitation is done.
+ `bool smtk::resource::Manager::empty() const` — to verify the manager has no resources.
+ `std::size_t smtk::resource::Manager::size() const` — to get a count of resources (but be aware this cannot
  be safely used for iteration of any sort since the size may be changed by other threads after the method
  returns. This method is intended mainly as a convenience for unit tests.

Note that registering/unregistering resource types (i.e., `smtk::resource::Metadata`) is still not threadsafe.

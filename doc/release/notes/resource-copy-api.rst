Resource System
===============

Copy API
--------

The :smtk:`Resource <smtk::resource::Resource>` class now has virtual methods to
produce an empty ``clone()`` of itself; to copy user data (via ``copyInitialize()``;
and to copy internal/external relationships among components (via ``copyFinalize()``).
See the user's guide for more information.

A new operation, :smtk:`CopyResources <smtk::operation::CoypResources>` is provided
that invokes these methods on a collection of resources.

Developer changes
~~~~~~~~~~~~~~~~~~

If you maintain your own resource type, you must override the methods above
if you wish to support cloning, copying, and updating your resources.
You may also wish to

* Implement copying of renderable geometry in your resource's custom
  subclass of :smtk:`smtk::geometry::Geometry`.
  By default, no geometry is copied.
  However, if you use the :smtk:`smtk::geometry::Cache` class for your resource's
  backend – as is commonly done for SMTK's resource classes – copying is implemented.
  Note that the Cache now requires the `DataType` type-alias in your backend to
  be copy-constructible.

* Review any custom property types registered for your resources;
  if they are not copy-constructible,
  :smtk:`smtk::resource::Resource::CopyProperties` will not copy
  them; either you must implement a copy constructor or copy these
  property values yourself.

User changes
~~~~~~~~~~~~

SMTK now provides a "copy resources" operation that accepts a set of input
resources to duplicate.
If all of the resources provide support for copying, then a copy of each
one is constructed and added to the resource manager on completion.

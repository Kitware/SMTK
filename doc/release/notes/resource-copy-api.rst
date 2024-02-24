Resource System
===============

Copy API
--------

The :smtk:`Resource <smtk::resource::Resource>` class now has virtual methods to
produce an empty ``clone()`` of itself; to copy user data (via ``copyData()``;
and to copy internal/external relationships among components (via ``copyRelations()``).
See the user's guide for more information.

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

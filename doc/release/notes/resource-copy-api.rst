Resource System
===============

Copy API
--------

The :smtk:`Resource <smtk::resource::Resource>` class now has virtual methods to
produce an empty ``clone()`` of itself; to copy user data (via ``copyData()``;
and to copy internal/external relationships among components (via ``copyRelations()``).
See the user's guide for more information.

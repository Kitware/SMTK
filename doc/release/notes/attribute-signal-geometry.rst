Attribute Subsystem
===================

Signal Operations for Renderable Attribute Resources
----------------------------------------------------

Several obstacles to providing renderable geometry for
attribute resources (and their components) have been removed.

The :smtk:`smtk::attribute::Signal` operation now marks
modified, created, and expunged components so their geometry
will be updated (but only if the resource has a registered
geometry object).

Also, the :smtk:`pqSMTKResource` class no longer skips
Signal operations when updating attribute resource
representations if the resource provides any geometry object.

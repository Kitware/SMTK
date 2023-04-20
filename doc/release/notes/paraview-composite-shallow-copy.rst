ParaView Changes
----------------

Shallow copy changed
~~~~~~~~~~~~~~~~~~~~

The implementation of ShallowCopy for composite datasets changed in VTK such
that SMTK's ``COMPONENT_ID`` information-key was not preserved, making UUIDs
unavailable to consumers of renderable geometry. We fix this by using the
original ShallowCopy implementation, which was renamed to CompositeShallowCopy.

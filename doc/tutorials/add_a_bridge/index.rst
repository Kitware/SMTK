============================
Bridge a new modeling kernel
============================

.. highlight:: c++
.. role:: cxx(code)
   :language: c++

This tutorial covers how to bridge a solid modeling kernel to SMTK.
The details will vary according to the capabilities of the modeling
kernel you wish to use via SMTK, but the overall process of bridging
the kernel involves

* subclassing SMTK's :smtk:`Bridge` class,
* defining a map between your kernel's modeling entities and SMTK UUIDs,
* transcribing information about kernel modeling entities into an
  SMTK model manager, and
* providing SMTK operators which perform modeling operations. The only
  mandatory operator is a "read" operator used to load a file native
  to your modeling kernel into your native kernel's modeling session.

This tutorial will use a simplistic bridge that presents an Exodus
mesh as a model composed only of groups (element blocks, side sets,
and node sets).
A bridge like this is useful for cases where the geometry for a
simulation has been completely prepared and SMTK is only being
used to attach attributes to pre-existing subsets of the geometric
model.
The groups themselves have a tessellation (i.e., a graphical representation)
based on the cells they contain but the groups do not expose any of
these cells to SMTK.

.. toctree::

   entity_uuids.rst
   bridge_subclass.rst
   transcribing.rst
   operators.rst
   troubleshooting.rst

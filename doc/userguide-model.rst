**********************
Overview of SMTK Model
**********************

SMTK's second major component is its geometric modeling system,
which provides bridges to multiple solid modeling kernels.

Model Entities
==============

The model manager is the only place where model topology and geometry are stored.
However, there are cursor-like classes, all derived from :cxx:`smtk::model::Cursor`,
that provide easier access to model traversal.

it is often necessary for the modeling kernel to live in a different process than other portions of
the simuation pipline.

.. figure:: figures/cursor-classes-with-inheritance.svg

   Each of the orange, green, purple, and red words is the name of a cursor class.
   The black arrows show relationships between instances of them (for which the
   classes at both ends provide accessors).

Remote models
=============

For many reasons (e.g., incompatible library dependencies, licensing issues, distributed processing),
it is often necessary for the modeling kernel to live in a different process than other portions of
the simuation pipline.

.. figure:: figures/forwarding-bridge.svg

   The CMB client-server model uses SMTK's RemoteOperator and DefaultBridge classes to
   forward operations from the client to the server (and results back to the client).

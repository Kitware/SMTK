Session: CGM
------------

SMTK has a *CGM* session type that bridges the `Common Geometry Model (CGM) <CGM>`_ meta-modeling kernel.
Depending on how your CGM library is compiled, it provides access to the ACIS and/or OpenCascade CAD kernels,
which are parametric modeling kernels that provide a full boundary-representation topology.

CGM also includes a discrete modeling kernel of its own focused on 3-dimensional polyhedral solid models.
While multiple CAD kernels may be used in the same CGM session, this is not the usual case.

.. todo:: The CGM session provides 2 "static" settings controlling the accuracy of model entity tessellations.
.. todo:: Starting CGM with a different Engine
.. todo:: CGM engines and file types

.. _CGM: http://sigma.mcs.anl.gov/cgm-library/

============================
Bridge a new modeling kernel
============================

.. contents::
.. highlight:: c++
.. role:: cxx(code)
   :language: c++

This tutorial covers how to bridge a solid modeling kernel to SMTK.
The details will vary according to the capabilities of the modeling
kernel you wish to use via SMTK, but the overall process of bridging
the kernel involves

* defining a map between your kernel's modeling entities and SMTK UUIDs
* transcribing information about kernel modeling entities into an
  SMTK model manager
* providing SMTK operators which perform modeling operations. The only
  mandatory operator is a "read" operator used to load a file native
  to your modeling kernel into your native kernel's modeling session.

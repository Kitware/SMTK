.. _smtk-overview:

-------------------------------
An Overview of SMTK's Subsytems
-------------------------------

SMTK's core library contains several subsystems,
each of which is covered in depth in sections that follow this one.
These subsystems are:

* The **resource** system holds a set of base classes used by the systems below to provide
  basic provenance information about versions of data on disk.
  Each file is a *resource*, which may hold a collection of *resource components*;
  the *resource manager* assembles resources together for processing into a simulation input.
* The **attribute** system, which provides a way to specify how information should be
  organized for scientific and engineering workflows, accept that information from users,
  and ensure that it is consistent with the specification.
* The **model** system, which provides geometric modeling and allows you to tie
  information from the attribute collection to geometric entities (e.g., assign boundary conditions
  in the attribute collection to particular boundaries on a CAD model).
* The **mesh** system, which can manipulate meshes of geometric models; it provides a way
  to propagate simulation attribute information from model entities onto meshes.
  It also provides a way to run external mesh creation tools on the model.
* The **simulation** (also known as the **export**) system, which is a set of utilities
  that lets you convert resources (e.g., attribute, model, and mesh collections) into
  an input deck for a simulation using Python scripts (or C++ if you wish).
* A **common** system holding utility classes.
* Python **bindings** that enable SMTK to
  be used *by* python scripts *and* SMTK to run python scripts as part of its normal operations.
* A set of **extension** libraries, which provide additional functionality but also introduce
  dependencies on software beyond what the core smtk library already requires.
  These are used to create applications built on SMTK.

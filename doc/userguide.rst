=================
SMTK User's Guide
=================

The Simulation Modeling Tool Kit, or SMTK, is a framework
to help you (1) describe a model in enough detail that it can
be passed to a solver and (2) create input files for a variety
of solvers in different simulation packages using your description.

This process can involve any or all of the following:

- importing a geometric model of the simulation domain or the domain's boundary;
- assigning sizing functions to specify mesh element size for analysis;
- submitting portions of the simulation domain to be meshed for analysis;
- assigning material properties to regions of the simulation domain;
- assigning boundary conditions to portions of the simulation domain's boundary;
- assigning initial conditions to portions of the simulation domain or its boundary; and
- assigning global simulation properties such as convergence criteria.

SMTK provides an attribute system that can represent
arbitrarily structured data and a template system that
can be tailored to describe almost any simulation's
expected inputs without allowing spurious or invalid
input specifications.
Thus, in addition to the process above, SMTK provides a
way for simulation packages to expose settings to
domain experts that makes preparing valid inputs simple.

SMTK also provides a uniform interface to several different
solid modeling kernels for preparing discretizations of your
simulation domain.

.. toctree::
   :maxdepth: 2

   userguide-attribute.rst
   userguide-model.rst
   userguide-contributing.rst

.. role:: cxx(code)
   :language: c++

SMTK Attribute System
---------------------

SMTK's first major component is its attribute modeling system,
which provides a way to model and represent non-geomtric
simulation data such as material properties, boundary conditions,
and solver parameters.
The attribute system has three main features:

1. An XML file syntax for specifying the kinds data to be modeled
for individual simulation codes and problem domains.
This file syntax is used to specify the unique inputs for a
simulation code, including, for example, data types, valid ranges,
and default values.
Specific data can be associated with geometric model entities,
and structural features can be used to
group and classify simulation data.


2. A set of user-interface panels for end users to create
and edit simulation data.
Using standard windows components (buttons, text boxes,
selection lists, etc.), users can enter all of the detailed
data needed to generate a desired simulation input.
The user-interface panels are automatically produced
by SMTK at runtime, based on the XML file(s) provided to the system.


3. An API for accessing the simulation data produced by end users.
Once an end-user has completed specifying the simulation data,
simulation-specific software can be used to traverse that data
and generate the simulation input files.
The native SMTK API is C++, and python bindings are also included.
In practice, python scripts are typically used to access the
simulation data and generate the simulation input files,
since no compiling is required.

.. system reads in a set of *definitions* specifying the data that
.. are relevant to each application.


.. will end up in simulation input decks for a given solver.

.. uses as its primary input a set of definitions

.. is configured for specific applications and problem domains
.. by a set of definitions

.. Since the simulation data are unique/specific to individual
.. problem and sovler domains,


Example Workflow
~~~~~~~~~~~~~~~~
SMTK can be used in a broad range of scientific and engineering
simulation applications.
In physics-based applications, for example,
structural mechanics, computational fluid dynamics, and
electromagnetic modeling, simulations are often performed relative
to a geometric model, which may have been created via
computer-aided design, or computationally constructed or
reconstructed from empirical data.
A discretization process is typically performed with the geometric
model to generate geometric input suitable for the simulation code,
for example, in the form of a finite element or finite volume mesh.
From there, the non-geometric inputs to the simulation code must
be generated, that is, the boundary conditions, material properties,
solver parameters, etc.

With SMTK, the process of generating this non-geometric input data
can be automated, providing benefits in reuse and error reduction.
Once the simulation-specific data are defined in an XML *template*,
domain experts or other end users can create the simulation data, or
*attribute data*, for specific problems using the SMTK user
interface panels.
Then a simulation-specific python script can be used to traverse
the attribute data and write the simulation input files.

Next:

* XML file example
* UI screenshot(s)



Attribute Types
~~~~~~~~~~~~~~~

User Interface Panels
~~~~~~~~~~~~~~~~~~~~~

Attribute Definitions
~~~~~~~~~~~~~~~~~~~~~

File Syntax (Reference)
~~~~~~~~~~~~~~~~~~~~~~~

API (reference)
~~~~~~~~~~~~~~~

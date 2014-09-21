*********************
First steps in Python
*********************

.. contents::
.. highlight:: python
.. role:: python(code)
   :language: python

To demonstrate the basic workflow in Python, we'll cover how to:

* create attribute and model managers;
* create an attribute system to hold simulation information;
* populate the attribute system with definitions for a deal.II program;
* generate an ensemble of input decks whose attributes use these definitions
  with different values as part of a sensitivity study;
* load a geometric model that has side sets used to hold boundary conditions;
* relate the side sets to boundary condition attributes in the ensemble; and
* write an input deck for each entry in the ensemble.

Our running example will be a fluid mechanics problem where
we run the ensemble to study the sensitivity of pump work
to viscosity and inlet velocity at our expected operating state.

Setup
=====

The first part of our script imports SMTK and creates managers:

.. literalinclude:: first_steps.py
   :start-after: # ++ 1 ++
   :end-before: # -- 1 --
   :linenos:

Problem Definition
==================

Now that the environment is set up, we can define the attribute
system that our simulation expects as its problem definition.

Everything in this entire section is usually replaced by creating an XML file
describing the problem definition.
However, the purpose of this tutorial is to demonstrate how to
implement your simulation workflow in Python and there can be times when
you wish to programmatically create a problem definition, or *template* file.

The first thing we do is create attribute :smtk:`Definitions <Definition>`
for the basic types of simulation inputs we must provide:

.. literalinclude:: first_steps.py
   :start-after: # ++ 2 ++
   :end-before: # -- 2 --
   :linenos:

Once we've created the material definition, we can add all the individual
properties required to specify a material.
More complicated definitions that allow materials of widely
different types — each with its own unique parameters — is a topic for another day.
Here, we just indicate that viscosity is the only parameter required to define our fluid,
perhaps because the simulation assumes incompressibility.

The definitions for boundary and initial conditions are more complex
because we might have different combinations of Dirichlet, Neumann, or Cauchy
conditions for different variables over different portions of the domain.
Rather than try to use a single :smtk:`Definition` for all of these,
we can use SMTK's inheritance system to create derived Definitions:

.. literalinclude:: first_steps.py
   :start-after: # ++ 3 ++
   :end-before: # -- 3 --
   :linenos:

We could also have created a definition to hold global simulation parameters
such as convergence criteria, halting criteria, and desired accuracy;
but for simplicity, we will assume the simulation package has reasonable
defaults.

Now that we have quantities of interest specified for each attribute definition,
we can provide hints to make data entry less error-prone.
For instance:

* SMTK accepts units for items that store continuous or discrete numeric values.
  These are presented to the user during entry to avoid confusion.
* Viscosity, temperature, and pressure have absolute minimum values.
  Items that store quantitative values accept minimum and maximum values;
  a boolean argument passed with the minimum or maximum indicates whether
  the limit value is attainable.
* Items store a single value in their given primitive type by default.
  But arrays of values may be accepted and these arrays may be fixed in size
  (as our 2-dimensional velocity requires exactly 2 components) or
  extensible between a minimum and maximum array length.
* It is also possible to mark an Item so that its entries should come
  from an enumerated set of acceptable values, but we do not illustrate
  that here.

.. literalinclude:: first_steps.py
   :start-after: # ++ 4 ++
   :end-before: # -- 4 --
   :linenos:

Now that our item definitions are constrained,
we can create attributes from the definitions.

Simulation Preparation
======================

The previous steps prepared a template that we can now use to
create input decks for multiple simulations.
The first step in creating input decks is to instantiate
attributes based on the definitions above:

.. literalinclude:: first_steps.py
   :start-after: # ++ 5 ++
   :end-before: # -- 5 --
   :linenos:

When you ask the :smtk:`Manager <smtk::attribute::Manager>` to
create an :smtk:`Attribute`, it provides an :smtk:`Item` to match
every :smtk:`ItemDefinition` in the attribute's underlying
:smtk:`Definition`.
These items are initialized to their default values (when a
default has been provided) and keep track of whether they have been
changed or not.
We will change the values of these attributes soon,
but first let's consider their relationship to the model geometry.

In the simplest workflow, you will already have a geometric
model of the simulation domain(s) and the domain boundaries
with the relevant groups of vertices, edges, faces, and/or
volumes named for use in applying boundary and initial
conditions.
Here, we read in an SMTK model session assuming that these
groups already exist with names that we know:

.. literalinclude:: first_steps.py
   :start-after: # ++ 6 ++
   :end-before: # -- 6 --
   :linenos:

Now we can loop over the parameters we wish to study and
create an ensemble for sensitivity analysis.

.. literalinclude:: first_steps.py
   :start-after: # ++ 7 ++
   :end-before: # -- 7 --
   :linenos:

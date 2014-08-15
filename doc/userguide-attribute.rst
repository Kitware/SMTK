*********************
SMTK Attribute System
*********************

General Description
===================

Background
----------

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
data needed to specify a desired simulation.
These user-interface panels are automatically produced
by SMTK at runtime, based on the XML file(s) provided to the system.


3. An API for accessing the simulation data produced by end users.
Once an end-user has completed specifying the simulation,
application software can be used to traverse that data
and generate the simulation input files.
The native SMTK API is C++, and python bindings are also included.
In practice, python scripts are typically used to access the
simulation data and generate the simulation input files.

.. system reads in a set of *definitions* specifying the data that
.. are relevant to each application.


.. will end up in simulation input decks for a given solver.

.. uses as its primary input a set of definitions

.. is configured for specific applications and problem domains
.. by a set of definitions

.. Since the simulation data are unique/specific to individual
.. problem and sovler domains,

Key Concepts
------------


Example Workflow
----------------

SMTK can be used in a broad range of scientific and engineering
simulation applications.
In physics-based applications, such as
structural mechanics, computational fluid dynamics (CFD), and
electromagnetic (EM) modeling, simulations are often performed relative
to a geometric model. This model may be created using
computer-aided design (CAD), or computationally
constructed/reconstructed from empirical data.
In either case, a
discretization process is typically performed with the
model to generate geometric input suitable for the simulation code,
often in the form of a finite element or finite volume mesh.
To complete the simulation inputs, the non-geometric inputs are
generated to enumerate the specific boundary conditions, material properties,
solver parameters, etc.

With SMTK, the process of generating simulation input data
can be automated, providing the
dual benefits of work-product reuse and error reduction.
Once the simulation-specific data are defined in an XML *template*,
domain experts or other end users can create the simulation data, or
*attribute data*, for specific problems using the SMTK user
interface panels.
Then simulation-specific python scripts can be used to traverse
the attribute data and write the simulation input files.

Next:

* XML file example
* UI screenshot(s)

.. class:: smaller-text
.. code:: xml

  <Definitions>
    <AttDef Type="Example1" Label="Example 1" BaseType="" Version="0"
            Unique="true" Associations="">
      <ItemDefinitions>
        <String Name="ExampleString" Label="String item:" Version="0"
                NumberOfRequiredValues="1">
          <BriefDescription>Enter some string of import</BriefDescription>
          <DefaultValue>Yellow denotes default value</DefaultValue>
        </String>
        <Int Name="ExampleInteger" Label="Integer item:" Version="0"
             NumberOfRequiredValues="1">
          <BriefDescription>For some integer value</BriefDescription>
          <DefaultValue>42</DefaultValue>
        </Int>
        <Double Name="ExampleDouble" Label="Double item:" Version="0"
                NumberOfRequiredValues="1">
          <BriefDescription>For floating-point precision values</BriefDescription>
          <DefaultValue>3.14159</DefaultValue>
        </Double>
        <Double Name="ExampleVector" Label="Double item w/3 values:" Version="0"
                NumberOfRequiredValues="3">
          <BriefDescription>Number of components is set to 3</BriefDescription>
          <ComponentLabels>
            <Label>x</Label>
            <Label>y</Label>
            <Label>z</Label>
          </ComponentLabels>
          <DefaultValue>0</DefaultValue>
        </Double>
        <String Name="SecondString" Label="Another string item:" Version="0"
                NumberOfRequiredValues="1">
          <BriefDescription>Enter some string of import</BriefDescription>
          <DefaultValue>whatever</DefaultValue>
        </String>
      </ItemDefinitions>
    </AttDef>

    <!-- Remaining content not shown -->

.. Wish I could align code & image horizontally

.. image:: figures/ExampleAttributePanel.png
   :align: center
   :width: 80%


Template File Syntax (Reference)
================================

File Layout
-----------

Advance Level Section <AdvanceLevel>?
-------------------------------------
tbd

Analysis Section <Analysis>
---------------------------
tbd

Attribute Section <Attributes>
------------------------------
tbd

Category Section <Categories>
-----------------------------
tbd

Definitions Section <Definitions>
---------------------------------

AttDef Element <AttDef>
^^^^^^^^^^^^^^^^^^^^^^^

XML attributes
""""""""""""""

Children elements
"""""""""""""""""

Item Definitions
^^^^^^^^^^^^^^^^

Double Item Definition <Double>
"""""""""""""""""""""""""""""""

XML attributes
~~~~~~~~~~~~~~

Children elements
~~~~~~~~~~~~~~~~~

File Item Definition <File>
"""""""""""""""""""""""""""
tbd

Group Item Definition <Group>
"""""""""""""""""""""""""""""
tbd

Integer Item Definition <Int>
"""""""""""""""""""""""""""""
tbd

String Item Definition <String>
"""""""""""""""""""""""""""""""
tbd

Void Item Definition <Void>
"""""""""""""""""""""""""""
tbd



RootView Section <RootView>
---------------------------
tbd

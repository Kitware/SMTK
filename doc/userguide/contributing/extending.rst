Extending SMTK
==============

As with all software, it is important to understand where
functionality you wish to add belongs: in your project,
in SMTK's core library, or in an upstream dependency of SMTK.

The tutorials provide in-depth guides on how to extend SMTK
in certain obvious directions,

* Writing an attribute resource template file to represent a solver's input format.
* Writing an exporter to support a new solver's input format.
* Adding a new solid-modeling operator
* Bridging SMTK to a new solid-modeling kernel

These tasks are all examples of projects that might use SMTK
as a dependency but not alter SMTK itself.
On the other hand, if you are attempting to provide functionality
that (a) does not introduce dependencies on new third-party libraries;
(b) will be useful to most projects that use SMTK; and
(c) cannot be easily be factored into a separate package,
then it may be best to contribute these changes to SMTK itself.
In that case, the rest of this section discusses how SMTK should be
modified and changes submitted for consideration.

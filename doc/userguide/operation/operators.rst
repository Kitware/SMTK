Operators
=========

In SMTK, operations performed on resources are represented
by instances of the :smtk:`Operator <smtk::operation::Operator>` class.
Each operation is a subclass that contains the code to perform the
operation by overriding the operate() and ableToOperate() methods.
These subclasses register themselves (via SMTK's auto-init macros)
with an :smtk:`Manager <smtk::operation::Manager>`.

.. todo:: Describe SMTK's auto-init macros.

The next sections describe in detail: first, how operators specify the inputs they require
and outputs they produce; and second, how operators register themselves for intropspection.

Inputs and Outputs
------------------

The inputs that an operator requires in order to run and
the outputs that an operator may produce are each modeled
as :smtk:`attribute definitions <smtk::attribute::Definition>`.
These definitions reside in an :smtk:`attribute resource <smtk::attribute::Resource>`.

Each operator's inputs are specified by a Definition that inherits a base attribute-definition
named "operator" while the its outputs are specified by a Definition that inherits a
base attribute-definition named "result".
Furthermore, if an operator's input specification is named "foo", its output specification
must be named "result(foo)".

SMTK uses this naming convention to construct results automatically.
Given an instance of an Operator subclass, you can call its
:smtk:`specification() <smtk::operation::Operator::specification>` method to obtain
the Attribute which models its inputs.
When the operator is executed, it returns an Attribute instance that models its result.

Recall that the attribute.Resource holds Definitions which contain ItemDefinitions.
When a Definition is instantiated as an Attribute, all its active ItemDefinitions
are instantiated as Items.
The Definitions themselves specify what types of model-entities may be
associated with their Attributes using a membership mask.
Associations between an operator's Attribute and model entities (i.e., subclasses of EntityRef)
denote the entities that the operator will act upon.
The operator-attribute's Items specify input parameters such as point locations,
geometric distances, etc.

The fact that inputs and outputs are specified using SMTK's own attribute resource
means that one need not construct an instance of the operator's C++ class in order
to obtain information about it;
instead, simply call :smtk:`operatorSystem() <smtk::model::Session::operatorSystem>`
on the session and ask for all the definitions which inherit "operator".

Registration
------------

* How to enumerate operators: ask the session.
* Operators are registered with an operation manager via the operator's use
  of the :smtk:`smtkDeclareOperator` and :smtk:`smtkImplementsOperator` macros.

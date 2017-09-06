Operators
=========

In SMTK, operations performed on geometric or conceptual models are represented
by instances of the :smtk:`Operator <smtk::model::Operator>` class.
Each operation, such as creating an edge, is a subclass that
contains the code to perform the operation by overriding the
operate() and ableToOperate() methods.
These subclasses register themselves (via SMTK's auto-init macros) in a way that allows
a list of applicable subclasses to be enumerated for any given type of model entity.
Operators perform work on a model *in situ*: they modify the model in place.
If you are familiar with VTK, this can be confusing, since VTK's pipeline filters
leave their input datasets untouched while SMTK's operators do not.
However, this in-place operation is a standard practice among solid modeling software.

.. todo:: Describe SMTK's auto-init macros.

The next sections describe in detail: first, how operators specify the inputs they require
and outputs they produce; and second, how operators register themselves for intropspection.

Inputs and Outputs
------------------

The inputs that an operator requires in order to run and
the outputs that an operator may produce are each modeled
as :smtk:`attribute definitions <smtk::attribute::Definition>`.
These definitions reside in an :smtk:`attribute collection <smtk::attribute::Collection>`
owned by an instance of a :smtk:`modeling session <smtk::model::Session>`.

Each operator's inputs are specified by a Definition that inherits a base attribute-definition
named "operator" while the its outputs are specified by a Definition that inherits a
base attribute-definition named "result".
Furthermore, if an operator's input specification is named "foo", its output specification
must be named "result(foo)".

SMTK uses this naming convention to construct results automatically.
Given an instance of an Operator subclass, you can call its
:smtk:`specification() <smtk::model::Operator::specification>` method to obtain
the Attribute which models its inputs.
When the operator is executed, it returns an Attribute instance that models its result.

Recall that the attribute.Collection holds Definitions which contain ItemDefinitions.
When a Definition is instantiated as an Attribute, all its active ItemDefinitions
are instantiated as Items.
The Definitions themselves specify what types of model-entities may be
associated with their Attributes using a membership mask.
Associations between an operator's Attribute and model entities (i.e., subclasses of EntityRef)
denote the entities that the operator will act upon.
The operator-attribute's Items specify input parameters such as point locations,
geometric distances, etc.

The fact that inputs and outputs are specified using SMTK's own attribute collection
means that one need not construct an instance of the operator's C++ class in order
to obtain information about it;
instead, simply call :smtk:`operatorSystem() <smtk::model::Session::operatorSystem>`
on the session and ask for all the definitions which inherit "operator".

The next sections go over conventions that SMTK uses for its inputs and outputs.

Inputs
^^^^^^

There are no *naming* conventions for input parameters, however:

* operators should use model-entity associations as the primary means
  for selecting geometry that an operator will act upon;
* also, by using the attribute collection to hold operator specifications,
  simply checking whether an attribute is in a valid state is usually enough to guarantee
  that the operator can run successfully.
  This is what the default implementation of ableToOperate() does.
  Subclasses may override the method to provide additional checks (i.e., whether a
  file is writable);
* when an operator needs only an associated set of model inputs,
  either because it has no parameters or they all take on valid default values,
  it can be run without further user input and applications are encouraged
  to provide a way to execute them immediately.
  However, when ableToOperate() returns false or applications wish to provide
  users a chance to override default parameters,
  some interface must be provided.
  The :smtk:`qtOperatorView <smtk::extension::qtOperatorView>` class is provided
  for this purpose.

Outputs
^^^^^^^

Output attributes have several items with standard names used to tell applications
what changes an operator has made to the model:

* ``created`` (:smtk:`ModelEntityItem <smtk::attribute::ModelEntityItem>`)
  An array of model entities that were created during the operation.
  These items may have tessellations (meaning there are new renderable entities)
  or relationships to existing entities (meaning there are new items to insert in the tree view).
* ``modified`` (:smtk:`ModelEntityItem <smtk::attribute::ModelEntityItem>`)
  An array of model entities that were modified during the operation.
  This does **not** imply that an entity's tessellation has changed;
  the ``tess_changed`` entry below is used for that.
* ``expunged`` (:smtk:`ModelEntityItem <smtk::attribute::ModelEntityItem>`)
  An array of model entities that were removed entirely from the model manager during the operation.
* ``tess_changed`` (:smtk:`ModelEntityItem <smtk::attribute::ModelEntityItem>`)
  An array of model entities whose geometric tessellations changed during the operation.
  This is signaled separately from ``modified`` above to minimize the overhead in
  rendering when only topological changes have occurred.
* ``cleanse entities`` (:smtk:`VoidItem <smtk::attribute::VoidItem>`)
  When present *and enabled*, this operator marks the ``modified`` and ``created`` entities
  as "clean" (meaning that they do not need to be saved; they are at exactly the state
  present in their owning-model's URL).
* ``allow camera reset`` (:smtk:`VoidItem <smtk::attribute::VoidItem>`)
  When present *and enabled*, this operator will *allow* (but not force) the camera of the
  active render view to be reset. A reset will actually occur when no renderable entities
  existed prior to the operation but at least one renderable entity exists afterward.
  Operators which load data from files are encouraged to include this item in their result
  attribute while operators which let users create or modify entities interactively — especially
  through interaction in render-views — are discouraged from allowing camera resets.

Registration
------------

* How to enumerate operators: ask the session.
* Operators are registered with a particular session via the operator's use
  of the :smtk:`smtkDeclareModelOperator` and :smtk:`smtkImplementsModelOperator` macros.
* Operators registered with the base session are inherited unless the session's
  constructor prevents it explicitly.

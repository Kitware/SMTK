Operators
=========

In SMTK, operations performed on resources are accomplished with
instances of the :smtk:`Operation <smtk::operation::Operation>` class.
Each operation is a subclass that contains the code to perform the
operation by overriding the ``operate()`` and ``ableToOperate()`` methods.
These subclasses register themselves (via SMTK's auto-init macros)
with a :smtk:`Manager <smtk::operation::Manager>`.

Operations are the only places where persistent objects (components and resources)
should be modified, created, or destroyed.
This is because operations hold locks to the resources they accept as inputs and
may run in parallel in other threads.
By using the locking mechanism which operations provide, you avoid race conditions,
memory stomping, and many other issues encountered with parallel code.

All exceptions thrown inside an operation or an operation handler/observer (described
below) will be caught and logged as errors.
This is done to avoid situations where resource locks are never released because an
exception thrown inside these code segments.

The next sections describe in detail: first, how operators specify the inputs they require
and outputs they produce; and second, how operators register themselves for introspection
by applications and scripts.

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

Recall that an :smtk:`attribute resource <smtk::attribute::Resource>` holds
:smtk:`definitions <smtk::attribute::Definition>` which contain
:smtk:`item definitions <smtk::attribute::ItemDefinition>`.
When an attribute is instantiated from a definition, all its active item definitions
are instantiated as items.
The definitions themselves specify what types of
:smtk:`persistent objects <smtk::resource::PersistentObject>` may be
associated with their attributes using a list of pairs of specifier strings;
the first string in the pair specifies the type-name of resource while the second
string specifies constraints on what types of components are allowed.
Associations between an operation's parameters and persistent objects such as model
entities denote what objects the operator will act upon.
The operation's parameters may also contain attribute items that specify other
configuration information for the operation such as point locations,
geometric distances, etc. that determine how the operation will proceed.

The fact that inputs and outputs are specified using SMTK's own attribute resource
means that one need not construct an instance of the operator's C++ class in order
to obtain information about it;
instead, simply call :smtk:`operatorSystem() <smtk::model::Session::operatorSystem>`
on the session and ask for all the definitions which inherit "operator".

Handlers
--------

You may add and remove handlers to an instance of an operation.
A handler is a function to be invoked upon the next completion of
one instance of an operation object (regardless of whether the result
indicates success or failure).

Handlers are function objects with a signature similar to observers
except that they are only called upon completion of an operation:
handlers may not cancel the operation and are not passed an ``EventType``
(only the operation and its result).
Handlers, like observers, are invoked at a time when all the resource
locks required for the operation are held.

Handlers are invoked on the thread in which the operation is run (unlike
observers, which are invoked on the main/GUI thread in Qt applications).

Handlers are invoked only for the instance of the operation they are
added to; if you create multiple operations of the same type and add
a handler to one, only that instance will have the handler called.

Handlers are invoked zero or one times at most.
Operation handlers are removed each time the operation is invoked;
you are responsible for adding handlers for each invocation.
If an operation is canceled by an observer, the handler is removed
from the operation and not invoked.
It is acceptable for a handler to add itself to the operation which
invoked it (as the container of handlers is copied and cleared before
any handlers are invoked).

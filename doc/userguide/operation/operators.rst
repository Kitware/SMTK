Operators
=========

In SMTK, operations performed on resources are represented
by instances of the :smtk:`Operation <smtk::operation::Operation>` class.
Each operation is a subclass that contains the code to perform the
operation by overriding the ``operate()`` and ``ableToOperate()`` methods.
These subclasses register themselves (via SMTK's auto-init macros)
with a :smtk:`Manager <smtk::operation::Manager>`.

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

Sideband information
--------------------

Applications may need to pass information to or retrieve information from operations.
While technically applications could always modify their operations' input parameters
and output results, this is often inconvenient or aesthetically displeasing – especially
if an operation may be used in different ways by different applications.
Thus, SMTK provides a way to pass "side-band" or "out-of-band (OOB)" information
to and from operations.

For example, an operation may be used inside a script where user interaction is not
possible as well as inside an interactive client.
When run interactively, the operation may wish to indicate to the application
how to adjust camera parameters to focus on a modified component.
While the operation's definition could be modified to include camera information
in its result data, this is undesirable because it clutters the output with
information that is specific to the application's behavior rather than the
operation's behavior.
Also, there may be many operations that all need to provide this information
keeping all of their result definitions consistent with one another is not
easy to maintain.

In these cases, you may place application-specific information into an
:smtk:`smtk::common::Managers` object that you provide to the operation manager
via a shared pointer.
Any operation constructed by the operation manager will have access to
the Managers object and can fetch arbitrary data from it in a type-safe manner.
Be aware that when using asynchronous operation launchers, multiple operations
may simultaneously access data in the Managers object;
methods that may be accessed in this way must be re-entrant.

Groups
------

Operations specific to a modeling session often implement functionality
present in many different sessions using a consistent set of parameters.
SMTK provides a :smtk:`grouping mechanism <smtk::operation::Group>` so
that applications can find sets of operations that provide "like" functionality.

For example, a CAD modeling session and a discrete modeling session
will generally provide operations to import, export, read, and write data.
Each of these tasks has its own group of operations that constrain their
member operations in different ways.
Consider the task of importing data;
import operations all require the location of the data to be imported.
The :smtk:`smtk::operation::ImporterGroup` holds a list of
operations that can import files. It constrains operations added to the
group to have a ``filename`` item.

Registration
------------

Operators are registered with an operation manager via a Registrar class
that implements ``registerTo()`` and ``unregisterFrom()`` methods.
Besides informing the operation manager of classes implementing operations,
your ``registerTo()`` method should also add operations to groups as needed.

Finally, each operation class may have an icon.
To register an icon for your operation, your Registrar class
should provide a second ``registerTo()`` method that accepts a
:smtk:`view manager <smtk::view::Manager>`.
The view manager provides an ``operationIcons()`` method that you can
use to register an icon to your operation.
Icons are functors that take in a secondary (background) color and return
a string holding SVG for an icon that contrasts well with the background.

Launching operations
--------------------

You can always call the ``operate()`` method of an operation instance
to run it immediately and synchronously.
However, operations may take a long time to complete and should not
interfere with applications tending to user input in the meantime.
Because of this, operations may also run in a separate thread from
user interface components.

The job of running operations asynchronously is delegate to
:smtk:`operation launchers <smtk::operation::Launcher>`.
SMTK provides a default launcher and one specific to the Qt
library (which is discussed in the :ref:`smtk-qt-sys` section).
If you use a launcher to run an operation, instead of getting
an operation result object, you will receive
a ``std::shared_future<Operation::Result>``.
Shared futures provide a way for applications to check whether
an operation is complete as well as block until an operation
completes (if needed).

In order to avoid multiple operations from making simultaneous changes to
the same objects at once,
the input parameters are scanned and each involved resource
is read-locked or write-locked (depending on how the operation's parameters
are marked) as needed before the operation runs.
Thus, operations that run on separate resources may run simultaneously,
as may operations that only require read access to the same resource.
However, operations that require write access to the same resource will
be run sequentially.

Observing operations
--------------------

Regardless of whether an operation is run synchronously or asynchronously,
if it was created by an operation manager, the manager will invoke any
observers that have been registered with it at two points:

* When an operation is about to run, observers are invoked that may inspect
  the operation and its parameters. Any of the observers may cancel the operation
  at this point, so there is no guarantee that observing an operation
  before it is run will generate a second event.
* When an operation has run, observers are invoked that may inspect the
  operation, its parameters, and its result (which indicates success or failure
  and which may also contain additional information).

While in general these observations may occur on any thread,
most applications will force the observations to occur on the main thread
because user-interface toolkits are rarely re-entrant.

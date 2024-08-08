Operation support
=================

Besides the operations themselves, SMTK provides applications with support
for determining which operations are apropos, scheduling operations, and
responding to their results.

.. _operation-sideband-info:

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

Resource Locking
----------------

To prevent operations running in separate threads from modifying the same persistent objects
at the same time, each operation must lock the resources it uses.
SMTK provides both read- and write-locks for resources.
Locking is performed only at the granularity of resources, not components.
Lock acquisition should never occur on the user-interface thread of an application.
By default, operations inspect their input parameters and acquire locks for all the resources
mentioned (directly or via components they own) before the ``operateInternal()`` method is
called.
You may override this, but you are responsible to ensure resource-locking is consistent
and does not cause deadlocks or race conditions.

An operation that locks a resource for reading may run simultaneous with other operations
that hold read-locks, but when a write-lock is held, no other operations that lock the
same resource may run simultaneously (whether read- or write-locks are requested).

It is possible for an operation to:

1. synchronously run another operation internally as part of its processing.

  The default behavior of ``operate()`` can be altered by passing a value of `Operation::BaseKey`
  as the parameter. This value is returned by ``Operation::childKey()`` and is only accessible by
  classes that derive from ``Operation``. Any operation that passes the value returned from
  ``Operation::childKey()`` establishes itself as the "parent" operation and the nested operation
  is considered a "child". The following options are available for altering the default behavior of
  ``operate()``:

    .. list-table:: Locking behavior
      :widths: 10 40
      :header-rows: 1

      * - Option
        - Description

      * - LockOption::LockAll
        - All resource locks will be acquired before ``operateInternal()`` is called. Any resource
          locks already acquired by the parent operation will be shared with the child operation,
          thus allowing the child operation to acquire any additional resource locks required by it.
          The child operation will fail if it attempts to acquire a write lock on a resource that
          the parent operation already holds a read lock on. **This is the default option**.

      * - LockOption::ParentLocksOnly
        - No additional locks will be attempted to be acquired by the child operation. The child
          operation will fail if it does require more resource locks than its parent.

      * - LockOption::SkipLocks
        - No resource locking will be attempted by the child operation. **This is the legacy
          behavior**.

    .. list-table:: Observer behavior
      :widths: 10 40
      :header-rows: 1

      * - Option
        - Description

      * - ObserverOption::InvokeObservers
        - Observers will be invoked following parameter validation and again following
          ``operateInternal()``.

      * - ObserverOption::SkipObservers
        - No observers will be invoked. **This is the default option**.

    .. list-table:: Parameters behavior
      :widths: 10 40
      :header-rows: 1

      * - Option
        - Description

      * - ParametersOption::Validate
        - The parameters of the child operation will be validated to determine if
          ``operateInternal()`` can be called. **This is the default option**.

      * - ParametersOption::SkipValidation
        - No validation of the child operation's parameters will take place. **This is the legacy
          behavior**.

2. asynchronously launch another operation to run later in a separate thread.



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

.. _operation-hints:

Operation Hints
---------------

Generally, operations should avoid side effects (:ref:`operation-sideband-info`)
and aim to be purely functional (modifying or deleting only their inputs and
possibly creating new output). However, for applications to behave intuitively,
side effects are often desirable.
For example, if an operation lofts two input curves to create a surface, the
user might expect the application selection to hold the surface after the operation
rather than the input curves (allowing them to run an extrusion operation directly).

To address this apparent paradox, SMTK provides operation results with hints
that the application (rather than the operation) can use to update its state.
Each hint is an attribute that held in a :smtk:`smtk::attribute::ReferenceItem`
named "hints" in the operation's result.

.. list-table:: Operation result hints
   :widths: 10 40
   :header-rows: 1

   * - Hint
     - Description

   * - render focus hint
     - The camera will have its aim point adjusted to the center of the
       bounds of all the objects associated to the hint.

   * - selection hint
     - The selection will be modified according to the hint's "action"
       item (replace, add, subtract) using the "value" from the hint
       as the integer value in the map from the associated objects to
       selection values.

       If the "ephemeral" item is enabled, then the application should
       attempt to delete the associated objects when they are removed
       from the selection.

   * - browser scroll hint
     - Scroll the resource browser tree to the first appearance of the
       first object associated to this hint.

   * - browser expand hint
     - Expand the resource browser tree items to show all occurrences
       of each of the objects associated to the hint.

There are several free functions in ``smtk/operation/Hints.h`` that you
can use to add hints to your operation and inspect hints in your
application's operation-observers.

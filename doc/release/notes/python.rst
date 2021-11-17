Updated Python bindings
-----------------------

Python bindings for accessing properties of resources and components
have been added; the bindings for the task system have been fixed
and are now tested.

If you have a python module that defines classes which inherit
``smtk.operation.Operation``, you can register all of these
operation classes to an operation manager by calling
a new ``registerModuleOperations`` method on the manager (passing
the module as a parameter).

There is now a python binding for ``smtk.extension.paraview.appcomponents.pqSMTKBehavior``.
You may call its ``instance()`` method in the ModelBuilder or ParaView python shell
and use the returned instance to obtain managers (using the ``activeWrapperResourceManager``,
``activeWrapperOperationManager``, ``activeWrapperViewManager``, and
``activeWrapperSelection`` methods).


Operation Tracing
-----------------

Operations executed in the gui are added to any active python trace. The first
operation adds imports and retrieves the managers used to replay an
operation. Operation inputs and associations are recorded in an XML string so
they are complete. Users should be able to copy-paste or replay the trace in
the internal python shell.

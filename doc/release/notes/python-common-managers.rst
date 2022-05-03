Python bindings for smtk.common
-------------------------------

Now `smtk.common.Managers` has python bindings.
Without this class being wrapped, it was impossible
to call `smtk.resource.Manager.read` and `smtk.resource.Manager.write`
since these methods now require a Managers instance.

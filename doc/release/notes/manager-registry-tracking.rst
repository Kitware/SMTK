Manager/registry tracking
=========================

Tracking of registrations between registrars and managers should now only be
done through a held ``Registry`` instance as returned by the
``Registrar::addToManagers`` method. In the future, direct access to
``Registrar::registerTo`` and ``Registrar::unregisterFrom`` will be disabled
and the ``Registry`` RAII object will be the only way to manage registrations.
This ensures that registrations are accurately tracked.

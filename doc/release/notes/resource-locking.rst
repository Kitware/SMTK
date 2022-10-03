Extended resource locking
-------------------------

Now any caller (not just the base Operation class) can acquire
resource locks via a new :smtk:`smtk::resource::ScopedLockSetGuard` class.
It has static ``Block()`` and ``Try()`` methods which accept a set of
resources to read-lock and a _set_ of resources to write-lock.
The method named ``Block()`` blocks until the locks are acquired
(NB: this may cause deadlocks if you are not careful).
The method named ``Try()`` does not block but may return a null pointer
to a ``ScopedLockSetGuard``.
If either method returns a non-null pointer, the referenced object's
destructor releases all the locks.
In no event will only a subset of resources be locked.

Task system
-----------

Parent resource method
~~~~~~~~~~~~~~~~~~~~~~

The `smtk::task::Manager::resource()` method has changed to return
a raw pointer. This is because we cannot construct a weak pointer
inside the constructor of any resource/project that will own a
task manager. Since it cannot be initialized at construction of its
parent, we have switched to holding a raw pointer.

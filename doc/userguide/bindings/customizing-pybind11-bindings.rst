.. _customizing-pybind11-bindings:

Customizing pybind11 bindings
=============================

One common shortcoming of the generated bindings is that templated methods
are not wrapped.
Sometimes, a templated method is the only way to set or get the state of
a C++ class instance and the template is used so that the various STL
containers can be used.
Consider the method :smtk:`smtk::resource::SelectionManager::modifySelection`
which is templated on the type of container holding objects to replace, be added to,
or be removed from the selection.
Without wrapping this method, it is impossible to change the selection
from Python.
By adding this definition to :file:`smtk/resource/pybind11/PybindSelectionManager.h`:

.. code:: c++

    PySharedPtrClass< smtk::resource::SelectionManager > instance(m, "SelectionManager");
    instance
        .def("modifySelection",
          (bool (smtk::resource::SelectionManager::*)(
            const ::std::vector<
              smtk::resource::Component::Ptr,
              std::allocator<smtk::resource::Component::Ptr> >&,
            const std::string&,
            int,
            smtk::resource::SelectionAction))
          &smtk::resource::SelectionManager::modifySelection)

we cast the function pointer to be wrapped to a particular template type that we
know pybind can handle (a vector of shared pointers to components).
While these casts of member functions can be verbose and sometimes difficult to read,
they make it easy to expose templated member functions to pybind without changing
the C++ class declaration, which is very desirable.

.. todo:: Explain how to make this robust against re-runs of the script that generates bindings.

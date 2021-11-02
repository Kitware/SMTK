.. _smtk-bindings:

---------------
SMTK's Bindings
---------------

SMTK is written in C++ and uses pybind11_ to generate python bindings.
Because SMTK also provides extensions to VTK and ParaView – which have
their own bindings which do not use pybind11 – SMTK's python bindings
may sometimes return these objects wrapped using other techniques in
order to support interoperability.

When you build SMTK with python support enabled (``SMTK_ENABLE_PYTHON_WRAPPING``),
you must supply a python library and interpreter.
SMTK's python bindings use this library's API and ABI, so ``import smtk``
will only work with this interpreter.
If you enable python support and ParaView support, then
SMTK and ParaView must use the same exact python library and interpreter.
In this case, you can use SMTK inside ParaView's python shell
and in scripts run by the ``pvpython`` executable shipped with ParaView.

Using bindings generated with one interpreter from a different
interpreter is unsupported and will generally cause failures that may be
hard to detect due to differences in API/ABI and/or compilation flags.

.. toctree::
   :maxdepth: 3

   python-overview.rst
   generating-pybind11-bindings.rst
   customizing-pybind11-bindings.rst

.. _pybind11: http://pybind11.readthedocs.io

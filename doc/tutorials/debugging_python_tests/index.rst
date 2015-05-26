**********************
Debugging Python Tests
**********************

.. contents::
.. highlight:: python
.. role:: python(code)
   :language: python

If a Python test is failing or if you just want to run part of a
test in order to perform some other task (like create a new test
using the interpreter), knowing how to invoke a test from within
the Python interpreter is useful.
This tutorial demonstrates how to run part of the SMTK discrete
session's "discreteReadFile" test.

Import the test module
======================

The first step is to import the test module and prepare
smtk.testing as required.
The test module is in the SMTK source directory, which is not
normally in your Python path.
For this tutorial, we'll assume the SMTK source is in :file:`/src`.

.. literalinclude:: debug.py
   :start-after: # ++ 1 ++
   :end-before: # -- 1 --
   :linenos:

Instantiate the test
====================

Normally a test is derived from `smtk.testing.TestCase` (in turn
derived from Python's `unittest.TestCase` class),
whose constructor requires a string argument specifying the name of
a single test to run.
Since we want to run the `readTest` method in the interpreter, we'll
pass that to the constructor:

.. literalinclude:: debug.py
   :start-after: # ++ 2 ++
   :end-before: # -- 2 --
   :linenos:

As you can see in the example above, it is easy to examine (and modify)
members that the test `setUp()` method creates.
By using the unittest framework in Python, tests can be used as
starting points when trying to reproduce bugs, when testing
new functionality (such as a new operator), or when generating
a related test interactively.

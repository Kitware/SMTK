Testing
=======

Testing is important to keep SMTK functioning as development continues.
All new functionality added to SMTK should include tests.
When you are preparing to write tests, consider the following

* Unit tests should be present to provide coverage of new classes and methods.
* Build-failure tests provide coverage for template metaprogramming by
  attempting to build code that is expected to cause static assertions or
  other compilation failures.
* Integration tests should be present to ensure features work in combination
  with one another as intended; these tests should model how users are expected
  to exercise SMTK in a typical workflow.
* Regression tests should be added when users discover a problem in functionality
  not previously tested and which further development may reintroduce.
* Contract tests should be added for downstream projects (i.e., those which depend
  on SMTK) which SMTK should not break through API or behavioral changes.
  A contract test works by cloning, building, and testing an external project;
  if the external project's tests all succeed, then the contract test succeeds.
  Otherwise, the contract test fails.

Unit tests
----------

SMTK provides a CMake macro named ``smtk_unit_tests`` that you should use to create unit tests.
This macro will create a single executable that runs tests in multiple source files;
this reduces the number of executables in SMTK and makes tests more uniform.
Because there is a single executable, you should make your test a function whose name
matches the name of your source file (e.g., ``int TestResource(int, const char* [])``)
rather than ``int main(int, const char* [])``.
The CMake macro also allows a LABEL to be assigned to each of the tests in the executable;
this label can be used during development to run a subset of tests and during integration
to identify areas related to test failures or timings in CDash.

Build-failure tests
-------------------

Build-failure tests verify that code which is expected to cause a compiler error
does actually cause an error.
A CMake macro named ``smtk_build_failure_tests`` is
provided in :file:`CMake/SMTKTestingMacros.cmake`.
This macro generates tests that succeed when they fail to compile code you provide, as a way
to improve coverage of template metaprogramming code.

You can attempt to build the same source file multiple times; each time, a compiler macro named
`SMTK_FAILURE_INDEX` is assigned an increasing integer so you can change what code is tested.
Consider this example

.. code-block:: c++

   int main()
   {
   #if SMTK_FAILURE_INDEX < 1
     static_assert(false, "Failure mode 1");
   #elif SMTK_FAILURE_INDEX < 2
     static_assert(false, "Failure mode 2");
     //...
   #endif

     return 0;
   }

If this file was named `simple.cxx`, you could add tests for it with

.. code-block:: cmake

   smtk_build_failure_tests(
     LABEL SomeLabel
     TESTS
       simple.cxx 2
     LIBRARIES smtkCore
   )

This would create 2 tests that each try to compile `simple.cxx`
with different `SMTK_FAILURE_INDEX` values (0 and 1).

Contract tests
--------------

Contract tests can be added by appending a URL to the :file:`SMTK_PLUGIN_CONTRACT_FILE_URLS`
CMake variable.
You can see an example of this in SMTK's top-level CMakeLists.txt:

.. literalinclude:: ../../../CMakeLists.txt
   :language: cmake
   :start-after: # ++ contract-example ++
   :end-before: # -- contract-example --
   :linenos:

Each URL listed in the CMake variable must be a CMake script;
typically, this script will declare a new project consisting of an external project dependent on SMTK.

.. literalinclude:: ../../../CMake/resource-manager-state.cmake
   :language: cmake
   :linenos:

When the script is run, the external project's source is downloaded, the project configured, and tests
are run. If they all complete with success, then the contract test succeeds.

This pattern of appending URLs was chosen so that machines used to test merge
requests which happen to have access to closed-source downstream projects could
be configured with additional URLs to contract tests. That way, even closed-source
downstreams can cause test failures in SMTK; this informs developers of SMTK merge
requests that additional work may be required on downstream projects before the
merge can succeed.

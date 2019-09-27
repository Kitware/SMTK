Testing
=======

Testing is important to keep SMTK functioning as development continues.
All new functionality added to SMTK should include tests.
When you are preparing to write tests, consider the following

* Unit tests should be present to provide coverage of new classes and methods.
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

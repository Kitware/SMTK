Add mechanism to include plugin contract tests to an SMTK build

We introduce the configure variable `SMTK_PLUGIN_CONTRACT_FILE_URLS`, a
list of URLs for contract files that each describe a plugin as an external
project. For each file in this list, a test is created that builds and
tests the plugin against the current build of SMTK. An example plugin
contract file, <CMAKE_SOURCE_DIR>/CMake/resource-manager-state.cmake,
has also been added as a prototype file for plugin contract tests.

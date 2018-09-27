## Support for SMTK as a third party submodule

This change replaces CMAKE_PROJECT_NAME with PROJECT_NAME within SMTK so
SMTK will build and install as a third party submodule of CMB. Also,
the "subproject" delineation was removed from cJSON (which will soon
be removed entirely), SMTKVTKExtensionMeshing, and
SMTKDiscreteModel. Finally, the header test macro has been modified to
accept as input the library associated with the header files.

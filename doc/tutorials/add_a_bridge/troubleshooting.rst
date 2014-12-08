Troubleshooting
---------------

If you have implemented a bridge but are having problems getting it
to run properly,

* Make sure that your Bridge constructor calls :smtk:`Bridge::initializeOperatorSystem`.
  Failure to do this will result in crashes when trying to
  create an operator, register an operator, or obtain a list of operator names.

* If you would like to run a model worker process in the debugger instead of
  having it started automatically by the process-local server created in
  :smtk:`RemusBridgeConnection::connectToServer`, you can set the
  :cxx:`SMTK_REMUS_MAX_WORKERS` environment variable to "0"
  — so that the maximum number of workers the server will start is 0.
  Started this way, the server will accept jobs but not have any available
  workers, so that when you start a worker that one is used.
  You can run the smtk-model-worker inside a debugger using a separate
  terminal window. For example,

  .. code:: sh

     # Is you use lldb:
     DYLD_FALLBACK_LIBRARY_PATH=/path/to/OCE/lib lldb -- \
     ./bin/smtk-model-worker \
       -kernel=cgm -engine=OpenCascade -rwfile=/path/to/cgm-occ.rw \
       -root=/path/to/root -site=ctest

     # Is you use gdb:
     DYLD_FALLBACK_LIBRARY_PATH=/path/to/OCE/lib gdb --args \
     ./bin/smtk-model-worker \
       -kernel=cgm -engine=OpenCascade -rwfile=/path/to/cgm-occ.rw \
       -root=/path/to/root -site=ctest

  will start a worker that can respond to requests
  made by the unitRemusBridgeConnection test.

*

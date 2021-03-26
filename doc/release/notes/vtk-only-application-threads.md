# VTK-only application and threads

An smtk application that uses Qt widgets, and only VTK and not Paraview can make two choices reguarding operations and threads:
* Run all operations on a single thread.
   * Set the cmake variable `SMTK_ENABLE_OPERATION_THREADS` to `OFF` when compiling SMTK, to disable threading.
* Forward signals from operation observers to the main thread:
   * After creating and populating `smtk::common::Managers` with the operation and resource manager, use
     `qtSMTKCallObserversOnMainThreadBehavior` to do this forwarding.

```
#include "smtk/extension/qt/qtSMTKCallObserversOnMainThreadBehavior.h"

auto *observersOnThread = qtSMTKCallObserversOnMainThreadBehavior::instance(win);
observersOnThread->forceObserversToBeCalledOnMainThread(m_managers);

```

Otherwise if operation observers try to change Qt state, unexpected behavior/crashes will result.

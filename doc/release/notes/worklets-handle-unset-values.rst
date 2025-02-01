Task System
===============================================================================================

Handle unset values
-------------------

Previously, the worklets panel would fail to fully populate when the result of a operation that
initializes the task system contains an unset value. A warning message will now be logged and
normal insertion of worklets will continue.

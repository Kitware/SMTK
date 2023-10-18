Bug Fixes Related to Tasks
--------------------------

Task::updateState
~~~~~~~~~~~~~~~~~

This method was not factoring in the task's internal state when determining either the task's current or new state which resulted in the graphical representation of the task node being incorrect.

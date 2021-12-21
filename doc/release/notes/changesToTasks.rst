Changed FillOutAttribute JSON Logic
-----------------------------------

Code now uses find instead of contains. This avoids doing double searches.

Task now supports Instance criteria
-----------------------------------
In addition to providing a list of Definitions names, you can now also specify a list of Attribute Names that also must be valid for the task
to be considered Completable.


Task now processes category changes on resources
-------------------------------------------------
A Signal operation identifying category changes now will trigger the task to recalculate its internal state.

Fixed Task State Calculations
-----------------------------

* If there are no resources found for the task, the state is now Unavailable
* If all of the task's definitions and instances are irrelevant then the task state is Irrelevant
* If any of the attributes required by the task are invalid, then the task is Incomplete
* Else the task is Completable

Added the following methods
---------------------------
* hasRelevantInfomation - returns true if it finds a relevant attribute or definition related to the task.  It will also determine if it found any attribute resources required by the task.

* testValidity - tests to see if an attribute is not currently in a ResourceAttributes entry.  If it is not then it's validity is tested, it is inserted into the appropriate set, and the method returns true, else it returns false.

Added smtk::tasks::Instances::findByTitle method
------------------------------------------------
Returns the tasks with a given title.

Improved TestTaskGroup
----------------------
The test now validates the states of the tasks after each modification.

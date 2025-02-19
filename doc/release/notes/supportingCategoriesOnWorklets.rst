Tasks
=====

Supporting Categories on Worklets
---------------------------------

You can now assign categories on Tasks Worklets.  A worklet's categories provides an additional conceptualization (similar to how categories are used by the attribute resource).  In addition, Tasks and the Task Manager can now uses this information to determine if the tasks generated from a worklet would make appropriate children of a task or appropriate top level tasks of the workflow.

Please see data/projects/SimpleWorkletExample/workletsWithCategories.smtk as an example.

API Changes to smtk::task::Worklet
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* setCategories(...)- assign a set of category strings to a worklet
* categories() - returns a set of category strings assigned to the worklet

Adding Category Expression Support to Task Manager
--------------------------------------------------

You can now associate a Category Expression to a Task Manager which will be used to determine if the tasks created by the worklet would be appropriate top-level tasks based on the worklet's categories. Currently the category expression is defined by the Manager's configuration.

**Note** Category evaluation for determining whether a task will accept a worklet as a child is only employed by the user interface to constrain options available to users; it is not enforced by the core SMTK library in either the task classes or the EmplaceWorklet operation.

API Changes to smtk::task::Manager
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* toplevelExpression() - returns a reference to a smtk::common::Categories::Expression

Adding Category Constraints to Tasks
------------------------------------

Tasks now can indicate if the tasks that would be created by a worklet would be appropriate as children tasks based on the set of categories associated with the worklet.  The way this has been implemented has been to add a method to task::Agents called *acceptsChildCategories* which will evaluate the set of categories and return one of the following results:

* CategoryEvaluation::Pass - the agent has determine a task with those categories would make an acceptable child task
* CategoryEvaluation::Reject - the agent has determine a task with those categories would not make an acceptable child task
* CategoryEvaluation::Neutral - the agent can not make a determination

When making a determination, the task will ask all of its agents to evaluate the categories.  If at least one agent rejects the categories, then the task will indicate they will not make appropriate children tasks.  Else, if at least one agent accepts the categories, the task will respond positively.  If all of the agents are neutral then the task will reject them.

**Note:** Category evaluation for determining whether a task will accept a worklet as a child is only employed by the user interface to constrain options available to users; it is not enforced by the core SMTK library in either the task classes or the EmplaceWorklet operation.

API Changes
~~~~~~~~~~~

smtk::task::Task
++++++++++++++++

* acceptsChildCategories(...) - returns true if none of its Agents reject the categories and at least one passes them.

smtk::task::Agent
+++++++++++++++++

* acceptsChildCategories(...) - returns the agent's evaluation of the categories.  Note that the base class will return ategoryEvaluation::Neutral.

Adding ChildCategoriesAgent
---------------------------

A new Agent class, :smtk:`ChildCategoriesAgent <smtk::task::ChildCategoriesAgent>`, has been developed to provide category expression support to a Task.  This agent provides a smtk::common::Categories::Expression which will be used when evaluating categores.  By default the expression is set to reject all categories.

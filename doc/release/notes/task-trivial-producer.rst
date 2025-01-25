Task System
===========

Agent for producing fixed port data
-----------------------------------

SMTK now includes a :smtk:`TrivialProducerAgent <smtk::task::TrivialProducerAgent>`
that can be configured to produce fixed data (in the form of :smtk:`smtk::task::ObjectsInRoles`)
on an output port.
This agent is intended to be used by subclasses of :smtk:`smtk::task::EmplaceWorklet`
that add new resources to projects along with the tasks of the worklet;
the worklet can have these newly-created resources (and/or components)
configured to appear on a top-level-task's output port using this agent
which simply returns an internal ``ObjectsInRoles`` object when asked for
port data.

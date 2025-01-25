Task System
===========

Task Diagnostics
----------------

Tasks and agents now collaborate to provide description and
diagnostic information for presentation to users.
The :smtk:`task <smtk::task::Task>` class can now have an
XHTML description to present information from workflow
designers to workflow users. The description is then combined
by the task's ``information()`` method with diagnostic XHTML
produced by :smtk:`agents <smtk::task::Agent::troubleshoot>`.

The amount of information provided by SMTK's current agents
is limited, but should be expanded over time.

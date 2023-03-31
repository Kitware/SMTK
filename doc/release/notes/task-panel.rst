User interface for task-based workflows
---------------------------------------

SMTK now provides a ParaView plugin which adds a :smtk:`task panel <pqSMTKTaskPanel>`
similar to ParaView's node editor. Each task is shown as a node in a graph and
may be made active, marked complete (or incomplete), and manually placed by the user.
Each task may have a collection of style keywords associated with it and the
task manager holds settings for these keywords that affect the application state
when matching tasks are made active.

In particular, SMTK's attribute-editor panel can be directed to display any view
held by attribute resource when a related :smtk:`smtk::task::FillOutAttributes` task
becomes active.

This functionality is a preview and still under development; you should expect
changes to user-interface classes that may break backwards compatibility while
this development takes place.

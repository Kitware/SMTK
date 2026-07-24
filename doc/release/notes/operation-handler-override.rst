Operation System
================

The base SMTK operation class now provides a method (``overrideHandlerInvocation()``)
that can be used to shift execution of operation handlers to a different thread than
the operation itself ran on.

This is useful inside GUI applications where many handlers may need to run on the GUI thread.
The existing :smtk:`pqSMTKCallObserversOnMainThreadBehavior` class has been updated to
forward handlers to the GUI thread in ParaView-based applications.

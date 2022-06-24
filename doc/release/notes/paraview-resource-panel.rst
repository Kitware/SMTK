ParaView resource panel
-----------------------

The :smtk:`pqSMTKResourcePanel` class now asks any :smtk:`smtk::view::ApplicationConfiguration`
present for view configuration before using a default. This makes it simpler for applications
to provide a custom phrase model, subphrase generator, or set of badges.
(Before this change, applications would have to wait for the panel to become ready and then
reconfigure it.)

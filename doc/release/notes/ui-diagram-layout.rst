Qt Extensions
=============

Node layout tools for diagram panel
-----------------------------------

The :smtk:`qtDiagram <smtk::extension::qtDiagram>` panel's pan and zoom
modes now provide tools to adjust the viewport (zoom to view the entire
diagram or just selected nodes) and node positions (align, distribute,
or lay-out selected nodes).

These tools make use of a new ``tools()`` method on qtDiagram that allows
modes to insert actions into the toolbar.

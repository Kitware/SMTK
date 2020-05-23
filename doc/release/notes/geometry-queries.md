## Geometry subsystem queries

The new geometry system in SMTK takes advantage of the
new query classes exposed in `smtk::resource::Resource::queries()`.
APIs are exposed for bounding boxes, closest point on discretization,
distance to (continuous) geometry, and "selection footprint."

The latter (selection footprint) is a query now used by the SMTK
representation to replace fixed-functionality methods that determined
what to highlight when a resource or component with no renderable
geometry was selected. This allows plugins that expose new resource
types to add their own logic.

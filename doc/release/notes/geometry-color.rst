Coloring renderable geometry
----------------------------

Before the property system was in wide use, the geometry
subsystem expected component colors to be passed via a
single-tuple field-data array on the renderable geometry.
Support for this was broken when support for coloring
by a floating-point property was added.

This commit fixes an issue (properly scaling floating-point
colors when generating an integer-valued color array) and
re-enables support for passing color by field-data.
Support for passing color by entity property is preserved,
but field-data arrays are preferred if present (because
presumably the geometry backend added this array).

This feature is being revived to support components inheriting
color from other components.

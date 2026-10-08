Attribute System
================

Fix duplicated units in double-item defaults
-------------------------------------------

Double-item defaults with degree units no longer gain a duplicate unit suffix
when read from strings or loaded from JSON. Both textual units such as
``degrees`` and the degree symbol (°) are preserved with a single suffix,
including through repeated JSON save and load cycles.

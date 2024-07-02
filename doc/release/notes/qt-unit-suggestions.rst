Qt extensions
=============

Double-item with unit completion
--------------------------------

The ``qtDoubleUnitsLineEdit.cxx`` class now uses the unit-library's
``PreferredUnits`` class to suggest unit completions. This allows
workflow developers to provide a tailored list of suggested completions
rather than listing every available compatible unit.

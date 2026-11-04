Boost 1.92 support
==================

Developer changes
-----------------

SMTK now supports building with Boost 1.92 while remaining compatible with
Boost 1.83. The UUID implementation explicitly includes its Boost integer and
random-number prerequisites instead of relying on transitive includes. The
project resource-container Python bindings now use SMTK's public iterator type
aliases rather than expanded Boost.MultiIndex implementation types.

Building SMTK with Boost 1.92 also resolves the ``TestIds`` abort encountered
with Boost 1.83 when using Xcode 27.

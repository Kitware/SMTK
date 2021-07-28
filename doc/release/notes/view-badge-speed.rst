Visibility badge improvements
=============================

The ParaView visibility-badge extension had an issue when large numbers
of phrase-model instances existed and a resource was closed: the visibility
was updated by completely rebuilding the map of visible entities which
is slow. This is now fixed.

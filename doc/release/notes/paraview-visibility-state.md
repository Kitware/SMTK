## Visibility badge inconsistency fixed

Closing and re-opening a resource that had some
component visibilities set to false (hidden) would
leave the visibility (eyeball) badge in an inconsistent
state. This has been fixed by forcing the badge to
reset its state each time a representation is removed
from the active view.

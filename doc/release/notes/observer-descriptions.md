## Descriptions for Observers

As one of its primary entrypoints, SMTK's Observer pattern enables
consuming projects to dynamically interact with SMTK objects. Because
Observers have the ability to change the functionality of an
application, they are a common site for debugging. We have added API
to Observers to describe an Observer as it is inserted, allowing for
the query of an observer's function during execution.

### Developer changes

Observers now accept an optional additional string for briefly
describing their function. Additionally, the ADD_OBSERVER macro has
been introduced to inject the insertion location as an Observer's
description.

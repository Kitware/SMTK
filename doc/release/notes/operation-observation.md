## Operation Changes

### Removal of "Operation Created" Event

Operation creation is often performed within observation events (so
the created operation can be added to the launcher queue), so it is
possible to deadlock ModelBuilder by repeatedly create operations when
an operation is running. Since no call sites specifically filter for
this event type, it has been removed.

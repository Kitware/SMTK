Eliminating the Need to Call Repopulate Root
--------------------------------------------

Previously, handling new persistent objects forced the Component Phrase Model to
call repopulateRoot which can be very expensive for large models.  The new approach
identifies those Resource Components that would be part of the Phrase Model's root subphrases and
properly inserts them in sorted order, eliminating the repopulateRoot() call.

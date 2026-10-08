Qt System
=========


Queue item view
---------------

Any string item may now have its view type set to ``qtQueueItem`` to
indicate that it should be set to either "active" (the default, indicating
that the active queue should be used) or the name of a queue present in the
application (which will depend on the set of plugins loaded whose registrars
declare job queues). Operations may then use this string-item's value to
fetch an appropriate queue at the time they create the job.

Reference Item
--------------

A bug in :smtk:`smtk::attribute::ReferenceItem`'s ``setNumberOfValues()`` method
sometimes modified an internal reference to the next unset but allocated value
pointing to a location that was unallocated. The next call to append a value
would then fail as the default append location was out of range. This has been
fixed and no action is necessary on your part. If you were observing failed
calls to append items to a ReferenceItem (or ComponentItem/ResourceItem), this
may have been the reason.

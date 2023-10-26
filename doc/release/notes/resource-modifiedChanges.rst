Resource System
---------------

Removing the MODIFIED Event from the Resource Manager
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It was determined that this event type was redundant since actions that would cause a Resource to be modified should be done via operations which would produce they own events.

In addition, it was observed that in some cases, operations that would change a Resource's **clean** state, would trigger the Resource's manager to emit its MODIFIED event which caused observer issues.

**Note:** The Resource::setClean method was the only thing that would explicitly cause the MODIFIED event to be emitted (though other methods do call setClean) and no longer does so.

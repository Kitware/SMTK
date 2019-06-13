# Resolved Issues
## Order Dependency Issue
The issue was observed in ModelBuilder where closing a resource that is associated with an attribute resource and then reloading the resource.  The issue was that the associations were not correct and pointed to the closed resource.

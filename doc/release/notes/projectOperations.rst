Changing Project I/O Operations
-----------------------------

Previously reading in the resources of a project used the read mechanism registered with the Resource Manager; however, resources loaded in this way may be processed differently than creating a reader via the operation manager. This happens because the operation manager typically has application-specific data in its ``smtk::common::Managers`` object and it provides this to operations it creates. In the case of attribute resources loaded by the resource manager, evaluators were not properly set while using the ReadResource approach did not have this issue.

These changes allow the ResourceContainer deserialization function to internally call the ReadResource Operation through the use of a operation::Helper which provides an operation::Operation::Key.

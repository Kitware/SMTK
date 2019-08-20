## Resource & Operation Observer signature

Instead of shared pointers, Resource and Operation Observers have been
updated to pass const references to Resources and Operations,
respectively. This change in signature has been made to help inform
downstream developers about the appropriate access privelages
associated with Resource and Operation observation (Observers should
observe, but not own, the things they observe).

### Developer changes

Because a downstream developer can acquire a shared pointer to a
Resource or Operation by calling `shared_from_this()`, the change is
largely syntactic. The change is made to guide the developer about
what he *should* do with these objects, not what he *can* do with
them.

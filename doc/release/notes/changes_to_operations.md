## Changes to Operations
### Removal of MarkModified operation
The MarkModified operation has been removed since it was considered redundant.  The attribute Signal operation should be used instead.

### operation::Manager: provide access to common::Managers

Operations frequently require access to other managers. This was
originally accomplished using inheritance to subset operations that
needed access to a manager type, and then modifying their create methods
to include access to the manager at runtime. With the introduction of a more
modular means to add managers to SMTK, this approach proved unscalable to
new manager types.

The new approach provides access to all managers through the operation
manager, if it is available. While scalable, the trade-off is the
requirement of an instance of smtk::common::Managers that holds available
managers.

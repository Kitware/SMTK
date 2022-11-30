Improvements for operation views
--------------------------------

Sometimes custom items need access to the operation view which created
them. This was not possible with :smtk:`smtk::extension::qtOperationView`
because internally it created a :smtk:`smtk::extension::qtInstancedView`
which owned the item. So, we add the operation view to the instanced-view's
configuration-information type-container and add a method so items can
fetch the configuration-information type-container from their parent view.

The new :smtk:`smtk::extension::qtOntologyItem` is an example of a
qtItem intended specifically for operation views that needs to reset
its state when its operation completes (and it needs to own the lock on
the operation parameters in order to do this so it does not change the
operation's configuration while the operation running).

The operation view now provides a ``disableApplyAfterRun`` property which
custom item views may set to indicate that re-running an operation
with identical parameters is valid (or, in the case of qtOntologyItem,
indicates the item cannot know when users have altered parameters due to
deficiencies in Qt).

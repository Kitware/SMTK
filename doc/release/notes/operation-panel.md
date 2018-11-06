# SMTK Operation Panel

SMTK now includes a paraview panel for editing model-operation
parameters (and then submitting the operation for execution).
Operations that appear are a subset of all operations;
which operations appear is determined by a whitelist of
operation names that is provided by the `OperationFilterSort`
class in a new `smtk/workflow` directory.
The subset of whitelisted operations that are available changes
in response to the active selection in the model tree view.

In more detail:
+ Add an operations panel UI.
+ Add a new smtk::view::AvailableOperators class that tracks
  a selection and an operation manager, calling observers when
  the list of operations changes (some work TBD).
+ Add `{observe,unobserve}Metadata` to smtk::operation::Manager so
  that new observers can be immediately passed the list of extant
  operation metadata.
+ Respond to selections in modelbuilder.
+ Add a `primaryAssociation()` method to operation metadata
  that returns the smtk::attribute::ReferenceItem for each
  operation (or nullptr if the operation's parameters don't
  support association).
+ Fix logic in AvailableOperators to test whether each
  operator accepts **all** the selected items and that
  the **number** of selected items is appropriate.
+ Remove some debug printouts and use the available-ops observer
  to print when the list changes.
+ Add a new subsystem to SMTK: **workflow**.
  The workflow system will host a task-dag used to provide
  suggestions to users. Currently, it exists only to
  filter+sort the list of applicable operations for
  presentation to the user.
+ Work on the AvailableOperations class in smtk/view to
  own an use the workflow operation filter.
+ Remove use of qtModelOperationWidget from custom operator views.
  This is in preparation for removal of qtModelOperationWidget,
  qtOperationDockWidget, qtOperationView, and qtModelView entirely.
  The qtModelView is replaced by the pqSMTKResourcePanel and the
  others will be replaced by the pqSMTKOperationPanel.
+ Add operation and resource managers to the qtUIManager.
  There may be a better place to put these that is not specific to Qt.
+ Test the workflow OperationFilterSort object (at least JSON
  deserialization).
+ Operation panel now lists available operations on the first selection change.
  The fact that it doesn't list an initial set of operations is a bug.
+ Fixed AvailableOperations so you can set its workflow filter.
+ Added methods to AvailableOperations to provide access to the
  user-facing data on operations that OperationFilterSort::Data provides.
+ Added the qtAvailableOperations widget. It is a subclass of QWidget
  and owns a QListWidget, which it populates with operations based on
  an AvailableOperations instance().
+ Add `createParameters` method to SpecificationOps.
+ When asked for operation parameters, always call `createParameters`
  rather than `extractParameters` so that operations always have unique
  parameters rather than sharing them.
+ Create a top-level view in the UI manager when none exists in order to
  display operators.
+ Listen for click events in the operation list widget and respond
  by showing a view of the matching operation.
+ Fix the XML reader to handle `AssociationDefs` that contain a
  `MembershipMask` tag.
+ Switch a bunch of operators to use `Accepts` entries instead of
  `MembershipMask`s in their associations (to be more specfic about what
  types of resources they accept).
+ Instantiate the pqPluginSMTKViewBehavior class inside
  pqSMTKAppComponentsAutoStart. Otherwise, custom views
  are not registered to the Qt UI manager.
+ The "name" of the assign colors operation has changed
  from "assign colors" to "smtk::model::AssignColors".
  Partially fix the custom view so that the UI is created.
+ Fix misspelled method.
+ Fix polygon "split edge" operation (only 1 edge can be split at a time).
+ Remove debug cruft.
+ Improve operator availability logic.
+ Refactor qtUIManager and pqSMTKOperationPanel.
  Now a qtUIManager may be constructed with an Operation instance
  whose attribute resource will be rendered as its view.
  Furthermore, applications can ask the qtUIManager to find/create a
  default view of its operation.
  This greatly simplifies the operation panel's job.
+ Rename `displayOperation()` methods in the operation panel to
  `editOperation()`, which is more accurate.
+ Fix `qtUIManager::initializeUI()` to create the proper type of
  view info when constructed with an operation.
+ Have the operation panel associate the current selection to the
  chosen operation when double-clicking.
+ Add an option to show **all** operations (or only those
  that match the current selection) to the AvailableOperations
  object. Also, expose it in the operation panel.
+ Have the operation view actually execute the operation,
  not just signal that it wants the operation run.
+ Make the polygon Write operation use associations
  for the resource it will write rather than an Item.
+ Make `smtk::view::Selection` hold `smtk::resource::PersistentObject::Ptr`s
  instead of component pointers. This will allow the operation
  panel to expose operations such as polygon::Write.
+ Add some example "workflow" files to the data directory.

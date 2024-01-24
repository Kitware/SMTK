Adding the Concept of UI Element State
--------------------------------------

SMTK provides :smtk:`smtk::view::Configuration` so that XML-formatted documents
(such as attribute XML files) can specify views for particular workflow tasks.
There are also times when application-provided user interface (UI) elements
(such as a panel or editor) needs to be configured at runtime with document-specific
JSON data.
For example when loading in a :smtk:`project <smtk::project::Project>`,
which contains a workflow of tasks that have been previously laid out in a :smtk:`diagram <smtk::extension::qtDiagram>`,
the diagram will need to be given this information when visually reconstructing the original graph.
Since this information is not part of the task, it should not be stored with the task JSON information.

To address this issue, SMTK has added a :smtk:`UIElementState <smtk::view::UIElementState>` class
that conceptually provides an API to configure the UI element's state represented as JSON and
to retrieve its current configuration as a JSON representation.

The UIElementState class is intended to be subclassed to produce and consume state data that is
relevant to a specific element in the application's user interface (e.g., panel, menu, etc.).

SMTK's :smtk:`view manager <smtk::view::Manager>` now holds a map from application UI element names
(provided by the application) to instances of classes derived from UIElementState.
Application UI elements should own an instances of UIElementState specific to itself
and insert it into (or remove it from) the view manager when the UI element is constructed
For example, an operation reading/writing a project may wish to read/write configuration of
the user interface by iterating over the view manager's map.

The first UI element to implement element state serialization is
the :smtk:`diagram panel <pqSMTKDiagamPanel>`, so that positions of task nodes
(as edited by users) can be saved and retrieved when writing and
reading projects that define those tasks.

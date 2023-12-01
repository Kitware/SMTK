Adding the Concept of UI Element State
--------------------------------------

There are times when a UI Element (such as a panel or editor) needs to be configured at runtime.  For example when loading in a Project, which contains a workflow of Tasks that have been previously laid out in a qtDiagram, the qtDiagram will need to be given this information when visually reconstructing the original graph.  Since this information is not part of the Task, it should not be stored with the Task JSON information.

To address this issue, SMTK has added the concept of a UIElementState class that conceptually provides an API to configure the UI Element's state represented as JSON and to retrieve its current configuration as a JSON representation.

The UIElementState class is intended to be subclassed to produce and consume state data that is relevant to a specific element in the application's user interface (e.g., panel, menu, etc.).

SMTK's View Manager now holds a map of instances whose classes are derived from smtk::view::UIElementState.

When an UIElementState object is constructed, it should be added to the View Manager.  An operation, for example reading/writing a Project, may need to configure or get the configuration of theses Elements.

The first class to be derived from this is pqSMTKTaskPanel so that Task Node locations can be saved and retrieved when writing and reading Projects respectively.

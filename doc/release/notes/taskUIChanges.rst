Changes to Task UI Architecture
-------------------------------

Subclassing task nodes
~~~~~~~~~~~~~~~~~~~~~~

Added the ability to assigned different types of qtTaskNodes to different tasks by making the following changes:

1. The original qtTaskNode class has been split into the following classes:
  * qtBaseTaskNode - an abstract base class from which all qtTaskNodes are derived from
  * qtDefaultTaskNode - an non-abstract class that functions as the original qtTaskNode class did.
2. Added the concept of a qtManager.  This is class's current role is to provide a qtTaskNodeFactory where plugins can added new qtTaskNode classes and the qtTaskEditor can find the appropriate qtTaskNode class for a specific Task.

To specify a qtTaskNode class for a Task, you can add the information in a Task Style as shown here:

    "styles": {
      "editPhysicalPropertyAttributes": {
        "attribute-panel": {
          "attribute-editor": "Physics"
        },
        "task-panel": {
          "node-class": "smtk::extension::qtDefaultTaskNode1"
        }
      },

qtTaskNode classes are specified w/r to the task-panel.

An additional qtTaskNode class : qtDefaultTaskNode1 has also been added as an example of creating a new qtTaskClass.  In this case, the window of the qtTaskNode is colored based on its state and activity.

Closing projects
~~~~~~~~~~~~~~~~

The task-manager user interface was not cleared when a project was closed.
This was due to the fact that the project manager's observers were not being invoked.
However, once this was corrected, several changes were required to
:smtk:`qtTaskEditor <smtk::extension::qtTaskEditor>` and
:smtk:`qtBaseTaskNode <smtk::extension::qtBaseTaskNode>` to properly remove nodes
and arcs from the scene:

+ Task nodes should not attempt to remove themselves from the scene inside their
  destructor; that is too late since overridden virtual methods to obtain their
  bounding boxes cannot be called from the destructor (causing a crash).
+ Instead, when the task editor wishes to clear the scene, it instructs
  the scene to delete items and then deletes its internal references to the items.

Assign colors operation
-----------------------

Previously, the AssignColors operation lived in the ``smtk::model``
namespace and could only be associated to model entities as it used
an API specific to model entities to set each component's color.
This operation has been generalized to use the base resource's
property API to set color and can thus be associated to any
persistent object. It now lives in the ``smtk::operation`` namespace.

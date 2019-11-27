Key Concepts
============

There are 2 ways that presentation is abstracted in SMTK:
views and trees of descriptive phrases.

:smtk:`View <smtk::view::Configuration>`
  instances are containers that hold information used to configure a view.
  There are currently 2 types of views in SMTK:
  * attribute views, which allow users to inspect and edit an entire attribute resource; and
  * phrase views, which allow users to inspect and edit any resource by interacting with
    a tree of one-line summary information related to a resource or component.

:smtk:`DescriptivePhrase <smtk::view::DescriptivePhrase>`
  instances represent a single piece of information that can be presented
  with a title string and an optional subtitle string.
  Phrases may be queried for a related resource, component, color, visibility,
  property name/value/type, and — most importantly — a list of child phrases
  that provide further details.

  Consider an example where we wish to present information about a model face.
  The face itself could be a phrase whose title is the user-provided name of
  the face, whose subtitle might indicate whether the face is planar or curved,
  whose color and visibility reflect user choices for display in a render window,
  and whose child phrases could include details about the face such as
  (1) a list of edges bounding the face,
  (2) properties defined on the face such as user annotation, and
  (3) attributes associated with the face such as boundary conditions.
  Developers may wish all or none of this information to be editable.

  Configuration information specifying how phrases should be arranged
  and what portions should be editable is held in a
  :smtk:`View <smtk::view::Configuration>` instance, since a view will hold
  the top of the phrase tree.

Besides views and descriptive phrases,
there are 2 important presentation tools that SMTK provides in its view system:

:smtk:`Selection <smtk::view::Selection>`
  instances hold a map from persistent objects to an integer value
  representing the "level" or "type" of selection that the object
  is participating in. For example, an object may be highlighted
  as a pointer hovers over it or selected more permanently.
  Different integer values indicate which (or both, if the integer
  is interpreted as a bit vector) of these types of selections an
  object belongs to.
:smtk:`AvailableOperators <smtk::view::AvailableOperators>`
  instances provide a list of editing operations that a user may perform.
  This list may vary based on the workflow as well as the current
  selection.

The following sections discuss portions of the view system in depth.

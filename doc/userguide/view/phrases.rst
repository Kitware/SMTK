Hierarchical Phrases
====================

Descriptive phrase trees may be flat or deeply nested.
They are flexible and may be configured to present the
same resources and components in many different ways,
while accommodating changes to those resources and components
in response to editing operations.

In order to deal with this complexity, the duties of phrases
are split across several classes:

* Phrase models are responsible for determining which
  descriptive phrases belong at the root of the tree.
  They also provide utilities used to
  * watch for events that may change the tree and
  * determine how to notify user interfaces of those
    changes in terms of deleted, added, re-ordered, and
    modified phrases.
  * mark each descriptive phrase with optional, clickable
    badge icons.
* Descriptive phrases are only responsible for storing
  child phrases and, optionally, a reference to a
  subphrase generator that can create or update
  the list of child phrases on demand.
  They also own a reference to another class
  responsible for the phrase's content (i.e., title,
  subtitle, and other related information).
* Subphrase generators create or update a list of
  child phrases when given a reference to a potential
  parent phrase.
* Phrase content classes own a reference to a persistent
  object and determine what about that object should be
  presented (i.e., they determine the topic of the phrase).
  One content class may hold a model face and present the face's
  name as the phrase title/topic, while another class may hold
  the same face but present one of the face's properties
  as its topic.

Phrase Models
-------------

.. todo:: Describe the base phrase model class

There are several phrase model subclasses:

:smtk:`ReferenceItemPhraseModel <smtk::view::ReferenceItemPhraseModel>`,
  which lists components available for use in a
  :smtk:`ReferenceItem <smtk::attribute::ReferenceItem>`.
  The top-level phrases it presents are persistent objects that either
  are or may be added to a ReferenceItem that you provide to the phrase
  model during configuration.
  It is used by the :smtk:`qtReferenceItem` class to present
  components that may be selected by a user in order to populate some item.

:smtk:`ResourcePhraseModel <smtk::view::ResourcePhraseModel>`,
  which lists resources matching a given set of filters at its top level.
  It is used by the :smtk:`pqSMTKResourcePanel` class to present
  a list of resources that have been loaded.

:smtk:`ComponentPhraseModel <smtk::view::ComponentPhraseModel>`,
  which lists components matching a given set of filters at its top level.

:smtk:`SelectionPhraseModel <smtk::view::SelectionPhraseModel>`,
  which lists objects held by a `Selection <smtk::view::Selection>`
  (which you provide at configuration time).
  By default, it will list all selected objects, regardless of the
  selection value. However, you may optionally specify a bit mask so
  that only objects whose selection values match the mask are shown.

Phrase Content Types
--------------------

.. todo:: Describe the base phrase content class and its children

Subphrase Generators
--------------------

.. todo:: Describe the base subphrase generator class and its children

Badges
------

Each phrase model owns a :smtk:`BadgeSet <smtk::view::BadgeSet>` used to
decorate phrases.
User interfaces that present phrases can ask the badge set for an array
of :smtk:`Badges <smtk::view::Badge>` that apply to a given phrase.
The returned array of badges will be ordered consistently.
Each badge has an SVG string to use as an icon, an optional tool tip,
and an "action" method used to perform some task when users click on
the badge.

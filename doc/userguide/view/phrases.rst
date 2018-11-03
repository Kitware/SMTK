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

  Phrase content classes may optionally reference another
  phrase content instance, which they decorate.
  For example, the :smtk:`VisibilityContent` class holds
  a reference to a :smtk:`ResourcePhraseContent`
  or :smtk:`ComponentPhraseContent` and simply overrides
  the visibility of the underlying resource or component
  with an application-specific visibility.

  Thus, each descriptive phrase holds a reference to the head
  of a singly-linked list of :smtk:`PhraseContent` instances
  that decorate the instance at the tail of the list.

Phrase Models
-------------

.. todo:: Describe the base phrase model class

There are two phrase model subclasses:

:smtk:`ResourcePhraseModel <smtk::view::ResourcePhraseModel>`,
  which lists resources matching a given set of filters at its top level.
  It is used by the :smtk:`pqSMTKResourcePanel` class to present
  a list of resources that have been loaded.

:smtk:`ComponentPhraseModel <smtk::view::ComponentPhraseModel>`,
  which lists components matching a given set of filters at its top level.
  It is used by the :smtk:`qtComponentItem` class to present
  components that may be selected by a user in order to populate some item.

Phrase Content Types
--------------------

.. todo:: Describe the base phrase content class and its children

Subphrase Generators
--------------------

.. todo:: Describe the base subphrase generator class and its children

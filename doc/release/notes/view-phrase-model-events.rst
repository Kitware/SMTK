View System
-----------

Phrase Model Ignores Most Resource-Manager Events
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With one exception in :smtk:`ResourcePhraseModel <smtk::view::ResourcePhraseModel>`,
the :smtk:`PhraseModel <smtk::view::PhraseModel>` classes now ignore resource-manager
events.
Instead, operation results are used to deal with the addition and removal
of resources to/from a resource manager.
This is consistent with the SMTK's paradigm: all modifications of resources and
components should take place inside operations and use the base
:smtk:`Operation <smtk::operation:Operation>` class to handle addition/removal of
resources to the application's manager.

If you have custom subclasses of PhraseModel, you should attempt to do the same.

As part of this change, the base PhraseModel class now provides a virtual
``processResource()`` method; it is invoked by the phrase-model's ``handleOperation()``
method when resources are added or removed by an operation.
Previously, both the ComponentPhraseModel and ResourcePhraseModel implemented methods
of the same name and signature that were **not** overrides of this new method in the
base class.
However, since they served the same purpose, they are now virtual overrides.
If you have custom subclasses of PhraseModel, you may wish to override this method
and you should guarantee that you do not hide the base-class method with a non-virtual
method of the same signature.

Using a Timer w/r PhraseModel's triggerDataChanged() method
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Put `PhraseModel::triggerDataChanged()` on a timer in order to prevent overly frequent redraws by rate-limiting observers to be fired at most 10 times per second.

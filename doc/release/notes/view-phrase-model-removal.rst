View System
-----------

Phrase Model Batches Triggers to Remove Phrases
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Previously, ``smtk::view::PhraseModel::removeChildren()`` would invoke
observers once for each phrase to be removed. Now, when phrases to be
removed are consecutive, a single invocation of the observers is
performed with a range of child indices. As long as your phrase-model
observers can handle ranges, no change should be required on your
part and performance should be improved for large removals.

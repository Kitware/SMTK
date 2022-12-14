Sort newly-created phrases
--------------------------

The default :smtk:`PhraseModel <smtk::view::PhraseModel>` implementation
of ``handleCreated()`` did not sort phrases to be inserted. This has been
corrected, but if your subphrase generator does not expect phrases to be
sorted by object-type and then title, this change will introduce improper
behavior and you will need to subclass this method to respond appropriately.

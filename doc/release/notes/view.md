## SMTK View System Changes

+ Changes to component names did not always result in
  reordering of phrases in the resource tree-view.
+ Reordering of phrases could result in a crash due
  to improper offsets passed to Qt's item model.
+ Now `smtk::view::PhraseContent` has a new internal
  variable to control the mutability of the top-level
  phrase's aspects (title, color, etc.). By default,
  everything is marked mutable (fixing a bug in
  ComponentPhraseModel), but you may change this by
  calling `setMutableAspects()` with a bit-flag taken
  from the values in the `PhraseContent::ContentType`
  enumeration.

## Phrase models

Several changes have been made to SMTK's descriptive phrase infrastructure:

+ A new subphrase generator that presents a fixed-depth tree
  is available and is the default.
+ Changes due to operations are now handled properly by the
  new subphrase generator but not by others.
+ There is a new phrase model, SelectionPhraseModel, that
  monitors a selection and populates the top-level phrases
  with selected objects.

### Developer-facing changes

Phrase models now properly listen for and respond to operation
manager events. When an operation completes, the phrase model
should respond by updating the list of top-level phrases and
then ask the top-level subphrase generator for a list of new
phrases to insert — along with the paths at which they belong.
The top-level subphrase generator may ask other subphrase
generators to add to this list.

Deleted and modified components are handled automatically by
the base phrase model class, but created components may appear
anywhere in the tree; the work of determining where to place
these phrases must be left to other classes.

## User-facing changes

The resource panel now presents the new two-level subphrase
generator.

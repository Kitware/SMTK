## Changes to the View System and Qt Extensions

### Changes to Processing DiscreteItems
#### Added "Please Select" option
In addition to showing all of the possible enum values, the qtDiscreteValueEditor will include a "Please Select" option.  This is used to show that the item is not set and can be used to unset the item.

#### Processing Category and Advance Level
Enums can now be filtered out based on the category and advance level information explicitly assigned to the enum.  If the item's current value is not considered "valid" based on the current category/advance level settings, it is added to the list but is colored red to indicate that it is not considered "valid".

### Changes to qtResourceBrowser

* Now inherits qtBaseView instead of QWidget, to allow configuration via a ViewInfo.
* A default .json config specifies the default PhraseModel and SubphraseGenerator types to use.
* The smtk::view::Manager class can dynamically construct PhraseModels and SubphraseGenerators based on typename.

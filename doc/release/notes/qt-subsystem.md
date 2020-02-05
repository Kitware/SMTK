## Changes to the View System and Qt Extensions

### Changes to Processing DiscreteItems
#### Added "Please Select" option
In addition to showing all of the possible enum values, the qtDiscreteValueEditor will include a "Please Select" option.  This is used to show that the item is not set and can be used to unset the item.

#### Processing Category and Advance Level
Enums can now be filtered out based on the category and advance level information explicitly assigned to the enum.  If the item's current value is not considered "valid" based on the current category/advance level settings, it is added to the list but is colored red to indicate that it is not considered "valid".

### Added a FixedWidth method to qtItem
A qtItem can now indicate if its width is fixed.  This is used by containers such as
qtGroupItem's Table to determine proper resize behavior

### Changes to qtResourceBrowser

* Now inherits qtBaseView instead of QWidget, to allow configuration via a ViewInfo.
* A default .json config specifies the default PhraseModel and SubphraseGenerator types to use.
* The smtk::view::Manager class can dynamically construct PhraseModels and SubphraseGenerators based on typename.
Example:
```
  // get the default config.
  nlohmann::json j = nlohmann::json::parse(pqSMTKResourceBrowser::getJSONConfiguration());
  smtk::view::ViewPtr config = j[0];
  // modify the PhraseModel type
  config->details().child(0).setAttribute("Type", "smtk::view::ComponentPhraseModel");
  // modify the SubphraseGenerator type
  config->details().child(0).child(0).setAttribute("Type", "smtk::view::TwoLevelSubphraseGenerator");
```

### Changes to Icons
#### Replaced Resource and Component icons with SVG representations
Icons for Resources and Components are now represented by SVG images, rather than PNG. The use of vectorized images makes icons scale with high retina desplays, and the programmatic instantation of these images allows for true color representations.

#### Added registration system for Resource and Component icons
Consuming applications can now register icon sets for Resources and Components, providing for more customization options and the use of icons that more closely reflect the entities they represent.
### Changes to qtSelectorView
* Now properly works with Analysis categories
* The view is considered empty if the selector item does not pass its category checks

### qtBaseView and qtBaseAttributeView Changes
* Made the following methods const
  * displayItem
  * isItemWriteable
  * advanceLevel
  * categoryEnabled
  * currentCategory

### qtAttribute Changes
* Added a removeItems methods - this removes all of the qtItems from the qtAttribute and allows createBasicLayout to be called again
* Added an option to createWidget that will allow a widget to be created even if the attribute's items are filtered out by categories and/or advanced level filtering.

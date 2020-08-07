## Changes to Qt View System

### Improving Observer Stability
It was discovered that passing the **this** pointer into an Observer's lambda expression in a Qt-based class can cause a crash if the object is deleted while there is an event still in the Qt Event loop.  The solution is to use a QPointer instead so that it can be tested for nullptr.

### Adding Observers to Views
* Observation for attribute creation, modification, and expungement have been added to qtAttributeView.
* Observation for attribute modification has been added to qtInstanceView

### Changes to qtBaseAttributeView
* CategoryTest was changed to take in a const attribute::ItemDefinitionPtr& instead of an attribute::ItemPtr since only the definition is needed.  This also eliminated the construction/destruction of a shared pointer.
* Added displayItemDefinition method which is similar to displayItem.
* isItemWriteable now takes in a const attribute::ItemPtr & instead of a attribute::ItemPtr which eliminates  the construction/destruction of a shared pointer.
* advaceLevelTest now takes in a const attribute::ItemPtr & instead of a attribute::ItemPtr which eliminates  the construction/destruction of a shared pointer.

### Changes to qtGroupView
* GroupBox Style icon has been changed from a check box to a closed/expand icon pair.  This change reduces confusion between optional items and viewing control widgets.
* Tabbed Group Views now show indicate invalid children views in their tabs using the alert icon

### API Changes
These changes were made to help simplify/cleanup the qtView infrastructure.  There were several places where onShowCategory() was being called in order to update the UI.  This resulted in confusion as to the role of the method.  In many cases these calls have been replaced with updateUI.

* **qtBaseView::updateViewUI - has been removed.** It was not being used.
* **qtBaseAttributeView::updateAttributeData - has been removed.** This method's role was to update the attribute content of a View.  You should now call updateUI() instead.
* qtBaseAttributeView no longer overrides updateUI()

### Tracking Changes in Analysis Configuration Attributes
Attributes that are deleted, created, or modified are now checked to see if they represent Analysis Configurations.  The configuration combobox is then updated appropriately.
